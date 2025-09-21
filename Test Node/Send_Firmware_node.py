import serial
import time
import struct
import os
from datetime import datetime

# Configuration
SER_PORT = "COM15"
SER_BAUDRATE = 9600
TIMEOUT = 0.06
SLEEP_TIME = 0.005
RESPONSE_TIMEOUT = 2.0

# OTA Protocol
SOF = 0xAA
EOF = 0xBB
PKT_TYPE_OTA = 0x02
OTA_TYPE_START = 0x00
OTA_TYPE_HEADER = 0x01
OTA_TYPE_DATA = 0x02
OTA_TYPE_RESPONSE = 0x03
OTA_TYPE_END = 0x04
OTA_TYPE_SIGNAL_UPDATE = 0x05

# STM32 Flash
STM32_FLASH_BASE = 0x08000000
STM32_FLASH_SIZE = 256 * 1024
STM32_PAGE_SIZE = 2048

# CRC32 Parameters
POLYNOMIAL = 0x04C11DB7
INITIAL_REMAINDER = 0xFFFFFFFF
FINAL_XOR_VALUE = 0xFFFFFFFF
WIDTH = 32
BITS_PER_BYTE = 8
TOPBIT = 0x80000000

def reflect(data, n_bits):
    reflection = 0
    for bit in range(n_bits):
        if data & (1 << bit):
            reflection |= (1 << ((n_bits - 1) - bit))
    return reflection

def crc_slow(p_message):
    remainder = INITIAL_REMAINDER
    for byte_val in p_message:
        remainder ^= (reflect(byte_val, BITS_PER_BYTE) << (WIDTH - BITS_PER_BYTE))
        for bit in range(BITS_PER_BYTE, 0, -1):
            if remainder & TOPBIT:
                remainder = ((remainder << 1) ^ POLYNOMIAL) & 0xFFFFFFFF
            else:
                remainder = (remainder << 1) & 0xFFFFFFFF
    return (reflect(remainder, WIDTH) ^ FINAL_XOR_VALUE) & 0xFFFFFFFF

def log_data(log_file, direction, frame_type, data, info=""):
    if not log_file:
        return
    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f")[:-3]
    hex_data = ' '.join(f'{b:02X}' for b in data)
    with open(log_file, 'a', encoding='utf-8') as f:
        f.write(f"[{timestamp}] {direction} {frame_type}\n")
        if info:
            f.write(f"     Info: {info}\n")
        f.write(f"     Data: {hex_data}\n")
        f.write(f"     Size: {len(data)} bytes\n\n")

def init_log(log_file):
    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    with open(log_file, 'w', encoding='utf-8') as f:
        f.write(f"=== STM32 OTA Log - {timestamp} ===\n\n")

def pack_header(session_id, fw_size, chunk_size, fw_crc, include_flash_addr=True, flash_addr=STM32_FLASH_BASE):
    total_chunks = (fw_size + chunk_size - 1) // chunk_size
    if include_flash_addr:
        # Format: >H H H H I I (6 items)
        return struct.pack(">HHHIII", session_id, fw_size, chunk_size, total_chunks, fw_crc, flash_addr)
    else:
        # Format: >H H H H I (5 items)  
        return struct.pack(">HHHHI", session_id, fw_size, chunk_size, total_chunks, fw_crc)

def build_frame(message_type, payload):
    header = struct.pack(">BBBH", SOF, PKT_TYPE_OTA, message_type, len(payload))
    crc_data = header + payload
    crc_val = crc_slow(crc_data)
    return header + payload + struct.pack(">IB", crc_val, EOF)

def wait_for_response(ser, timeout=RESPONSE_TIMEOUT, log_file=None):
    start_time = time.time()
    response_data = bytearray()
    print("    Waiting...", end="")
    
    while (time.time() - start_time) < timeout:
        if ser.in_waiting > 0:
            chunk = ser.read(ser.in_waiting)
            response_data.extend(chunk)
            if chunk:
                hex_chunk = ' '.join(f'{b:02X}' for b in chunk)
                ascii_chunk = ''.join(chr(b) if 32 <= b < 127 else '.' for b in chunk)
                print(f"\n    RX: {hex_chunk} | ASCII: {ascii_chunk}")
        time.sleep(0.01)
    
    if response_data:
        print(f"    Response: {len(response_data)} bytes")
        if log_file:
            log_data(log_file, "RX", "RESPONSE", response_data, f"After {time.time() - start_time:.3f}s")
        return bytes(response_data), True
    print("    No response")
    return bytes(), False

def hex_to_stm32_binary(hex_file, target_flash_addr=STM32_FLASH_BASE):
    print(f"[*] Converting {hex_file} -> STM32 binary (target: 0x{target_flash_addr:08X})")
    memory_map = {}
    extended_address = 0
    min_addr = max_addr = entry_point = None
    
    with open(hex_file, "r") as f:
        for line_num, line in enumerate(f, 1):
            line = line.strip()
            if not line or not line.startswith(":"):
                continue
                
            try:
                byte_count = int(line[1:3], 16)
                address = int(line[3:7], 16)
                record_type = int(line[7:9], 16)
                data_hex = line[9:9 + byte_count * 2]
                checksum = int(line[9 + byte_count * 2:11 + byte_count * 2], 16)
                
                # Verify checksum
                calc_sum = byte_count + (address >> 8) + (address & 0xFF) + record_type
                for i in range(0, len(data_hex), 2):
                    calc_sum += int(data_hex[i:i+2], 16)
                calc_sum = ((~calc_sum) + 1) & 0xFF
                
                if calc_sum != checksum:
                    print(f"[WARNING] Checksum error at line {line_num}")
                    continue
                
                if record_type == 0x00:  # Data Record
                    full_address = extended_address + address
                    if full_address >= target_flash_addr:
                        data_bytes = bytes.fromhex(data_hex)
                        for i, byte_val in enumerate(data_bytes):
                            memory_map[full_address + i] = byte_val
                        if min_addr is None or full_address < min_addr:
                            min_addr = full_address
                        if max_addr is None or (full_address + len(data_bytes) - 1) > max_addr:
                            max_addr = full_address + len(data_bytes) - 1
                elif record_type == 0x01:  # End of File
                    break
                elif record_type == 0x02:  # Extended Segment Address
                    extended_address = int(data_hex, 16) << 4
                elif record_type == 0x04:  # Extended Linear Address
                    extended_address = int(data_hex, 16) << 16
                elif record_type == 0x05:  # Start Linear Address
                    entry_point = int(data_hex, 16)
            except (ValueError, IndexError) as e:
                print(f"[ERROR] Invalid HEX at line {line_num}: {e}")
                continue
    
    if not memory_map:
        print("[ERROR] No valid data found!")
        return bytes(), 0, 0, 0
    
    # Create continuous binary
    bin_data = bytearray()
    for addr in range(min_addr, max_addr + 1):
        bin_data.append(memory_map.get(addr, 0xFF))
    
    print(f"[*] Range: 0x{min_addr:08X}-0x{max_addr:08X}, Size: {len(bin_data)} bytes")
    if entry_point:
        print(f"[*] Entry point: 0x{entry_point:08X}")
    
    return bytes(bin_data), min_addr, max_addr, entry_point or 0

def find_firmware_file():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    for ext in ['.hex', '.bin']:
        for f in os.listdir(script_dir):
            if f.endswith(ext):
                return os.path.join(script_dir, f)
    return None

def send_stm32_ota(hex_file=None, chunk_size=64, enable_log=True, target_flash_addr=STM32_FLASH_BASE, 
                   include_flash_addr=True, session_id=None, wait_response=True):
    
    if session_id is None:
        import random
        session_id = random.randint(0x1000, 0xFFFF)
    
    if hex_file is None:
        hex_file = find_firmware_file()
        if hex_file is None:
            print("[ERROR] No firmware file found!")
            return
    
    log_file = None
    if enable_log:
        script_dir = os.path.dirname(os.path.abspath(__file__))
        log_file = os.path.join(script_dir, "stm32_ota_log.txt")
        init_log(log_file)
        print(f"[*] Logging: {log_file}")
    
    try:
        fw_bin, start_addr, end_addr, entry_point = hex_to_stm32_binary(hex_file, target_flash_addr)
        if not fw_bin:
            print("[ERROR] No binary data extracted!")
            return
    except Exception as e:
        print(f"[ERROR] Failed to read hex file: {e}")
        return
    
    fw_size = len(fw_bin)
    if fw_size > 65535:
        print(f"[ERROR] Firmware size ({fw_size}) exceeds 65535 bytes!")
        return
        
    fw_crc = crc_slow(fw_bin)
    total_chunks = (fw_size + chunk_size - 1) // chunk_size
    
    print(f"\n[*] OTA Session 0x{session_id:04X}")
    print(f"    Size: {fw_size} bytes, CRC: 0x{fw_crc:08X}")
    print(f"    Flash: 0x{start_addr:08X}-0x{end_addr:08X}")
    print(f"    Chunks: {total_chunks} x {chunk_size} bytes")
    
    all_responses = []
    
    try:
        with serial.Serial(SER_PORT, SER_BAUDRATE, timeout=TIMEOUT) as ser:
            print(f"\n[*] Serial: {SER_PORT} @ {SER_BAUDRATE}")
            time.sleep(0.1)
            ser.reset_input_buffer()
            ser.reset_output_buffer()
            
            # START
            print("\n[1] START")
            frame = build_frame(OTA_TYPE_START, b"\x00")
            log_data(log_file, "TX", "START", frame, "Begin OTA")
            ser.write(frame)
            print("[TX] START")
            
            if wait_response:
                response, _ = wait_for_response(ser, RESPONSE_TIMEOUT, log_file)
                if response:
                    all_responses.append(("START_RESPONSE", response))
            
            # HEADER
            print("\n[2] HEADER")
            header_payload = pack_header(session_id, fw_size, chunk_size, fw_crc, include_flash_addr, start_addr)
            frame = build_frame(OTA_TYPE_HEADER, header_payload)
            info = f"session=0x{session_id:04X}, size={fw_size}, chunks={total_chunks}, crc=0x{fw_crc:08X}"
            log_data(log_file, "TX", "HEADER", frame, info)
            ser.write(frame)
            print(f"[TX] HEADER (session=0x{session_id:04X}, size={fw_size}, chunks={total_chunks})")
            
            if wait_response:
                response, _ = wait_for_response(ser, RESPONSE_TIMEOUT, log_file)
                if response:
                    all_responses.append(("HEADER_RESPONSE", response))
            
            # DATA
            print(f"\n[3] DATA ({total_chunks} chunks)")
            for i in range(0, fw_size, chunk_size):
                seq = i // chunk_size
                chunk = fw_bin[i:i+chunk_size]
                
                if include_flash_addr:
                    chunk_addr = start_addr + i
                    payload = struct.pack(">HI", seq, chunk_addr) + chunk
                else:
                    payload = struct.pack(">H", seq) + chunk
                
                frame = build_frame(OTA_TYPE_DATA, payload)
                info = f"seq={seq}/{total_chunks-1}, size={len(chunk)}"
                log_data(log_file, "TX", "DATA", frame, info)
                ser.write(frame)
                
                progress = (seq + 1) * 100 // total_chunks
                print(f"[TX] DATA {seq:3d}/{total_chunks-1} [{progress:3d}%] {len(chunk):2d}B", end="")
                if include_flash_addr:
                    print(f" @0x{start_addr + i:08X}")
                else:
                    print()
                
                if wait_response:
                    response, _ = wait_for_response(ser, RESPONSE_TIMEOUT * 0.5, log_file)
                    if response:
                        all_responses.append((f"DATA_RESPONSE_{seq}", response))
                
                time.sleep(SLEEP_TIME)
            
            # END
            print("\n[4] END")
            frame = build_frame(OTA_TYPE_END, b"\x01")
            log_data(log_file, "TX", "END", frame, "End data transmission")
            ser.write(frame)
            print("[TX] END")
            
            if wait_response:
                response, _ = wait_for_response(ser, RESPONSE_TIMEOUT, log_file)
                if response:
                    all_responses.append(("END_RESPONSE", response))
            
            # SIGNAL_UPDATE
            print("\n[5] SIGNAL_UPDATE")
            update_info = struct.pack(">II", start_addr, entry_point)
            frame = build_frame(OTA_TYPE_SIGNAL_UPDATE, update_info)
            log_data(log_file, "TX", "SIGNAL_UPDATE", frame, f"flash=0x{start_addr:08X}, entry=0x{entry_point:08X}")
            ser.write(frame)
            print(f"[TX] SIGNAL_UPDATE (flash=0x{start_addr:08X}, entry=0x{entry_point:08X})")
            
            if wait_response:
                response, _ = wait_for_response(ser, RESPONSE_TIMEOUT * 2, log_file)
                if response:
                    all_responses.append(("SIGNAL_UPDATE_RESPONSE", response))
            
    except Exception as e:
        print(f"[ERROR] {e}")
        return
    
    print(f"\n[*] OTA Complete! Session 0x{session_id:04X}: {fw_size} bytes sent")
    print(f"[*] Responses received: {len(all_responses)}")
    
    if all_responses and log_file:
        with open(log_file, 'a', encoding='utf-8') as f:
            f.write(f"\n{'='*60}\nRESPONSE SUMMARY - Session 0x{session_id:04X}\n{'='*60}\n\n")
            for i, (resp_type, resp_data) in enumerate(all_responses):
                hex_data = ' '.join(f'{b:02X}' for b in resp_data)
                ascii_data = ''.join(chr(b) if 32 <= b < 127 else '.' for b in resp_data)
                f.write(f"{i+1}. {resp_type}\n   HEX: {hex_data}\n   ASCII: {ascii_data}\n   Size: {len(resp_data)}B\n\n")

def test_crc():
    test_data = b"123456789"
    result = crc_slow(test_data)
    print(f"CRC test: '{test_data.decode()}' -> 0x{result:08X}")
    
    # Test STM32 vectors
    stm32_vectors = bytes([0x00, 0x20, 0x00, 0x20, 0xED, 0x00, 0x00, 0x08])
    crc_vectors = crc_slow(stm32_vectors)
    print(f"STM32 vectors CRC: 0x{crc_vectors:08X}")

def read_binary_file(bin_file):
    """Read .bin file directly"""
    try:
        with open(bin_file, 'rb') as f:
            data = f.read()
        print(f"[*] Binary file: {bin_file}, Size: {len(data)} bytes")
        return data, STM32_FLASH_BASE, STM32_FLASH_BASE + len(data) - 1, STM32_FLASH_BASE
    except Exception as e:
        print(f"[ERROR] Cannot read binary file: {e}")
        return bytes(), 0, 0, 0

def interactive_menu():
    """Interactive menu for OTA operations"""
    print("\n=== STM32 OTA Interactive Menu ===")
    print("1. Send OTA with auto-detected firmware")
    print("2. Send OTA with specific file")
    print("3. Test CRC calculation")
    print("4. Show configuration")
    print("0. Exit")
    
    while True:
        try:
            choice = input("\nSelect option (0-4): ").strip()
            
            if choice == '0':
                print("Goodbye!")
                break
            elif choice == '1':
                firmware = find_firmware_file()
                if firmware:
                    print(f"Using: {firmware}")
                    send_stm32_ota()
                else:
                    print("No firmware file found!")
            elif choice == '2':
                filename = input("Enter firmware file path: ").strip()
                if os.path.exists(filename):
                    chunk_size = int(input("Chunk size (default 128): ") or "128")
                    include_addr = input("Include flash address? (y/N): ").lower() == 'y'
                    session_id = input("Session ID (hex, default auto): ").strip()
                    if session_id:
                        session_id = int(session_id, 16)
                    else:
                        session_id = None
                    send_stm32_ota(filename, chunk_size, True, STM32_FLASH_BASE, include_addr, session_id)
                else:
                    print("File not found!")
            elif choice == '3':
                test_crc()
            elif choice == '4':
                print(f"\nCurrent Configuration:")
                print(f"  Serial Port: {SER_PORT}")
                print(f"  Baudrate: {SER_BAUDRATE}")
                print(f"  Flash Base: 0x{STM32_FLASH_BASE:08X}")
                print(f"  Flash Size: {STM32_FLASH_SIZE//1024}KB")
                print(f"  Page Size: {STM32_PAGE_SIZE}B")
                print(f"  Response Timeout: {RESPONSE_TIMEOUT}s")
            else:
                print("Invalid choice!")
        except KeyboardInterrupt:
            print("\nExiting...")
            break
        except Exception as e:
            print(f"Error: {e}")

def send_stm32_ota_enhanced(hex_file=None, chunk_size=64, enable_log=True, target_flash_addr=STM32_FLASH_BASE, 
                           include_flash_addr=True, session_id=None, wait_response=True, interactive=False):
    """Enhanced OTA function with additional features"""
    
    if session_id is None:
        import random
        session_id = random.randint(0x1000, 0xFFFF)
    
    if hex_file is None:
        hex_file = find_firmware_file()
        if hex_file is None:
            print("[ERROR] No firmware file found!")
            return False
    
    log_file = None
    if enable_log:
        script_dir = os.path.dirname(os.path.abspath(__file__))
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        log_file = os.path.join(script_dir, f"stm32_ota_{timestamp}.txt")
        init_log(log_file)
        print(f"[*] Logging: {log_file}")
    
    try:
        # Determine file type and process
        if hex_file.lower().endswith('.bin'):
            fw_bin, start_addr, end_addr, entry_point = read_binary_file(hex_file)
        else:
            fw_bin, start_addr, end_addr, entry_point = hex_to_stm32_binary(hex_file, target_flash_addr)
        
        if not fw_bin:
            print("[ERROR] No binary data extracted!")
            return False
            
    except Exception as e:
        print(f"[ERROR] Failed to process file: {e}")
        return False
    
    fw_size = len(fw_bin)
    if fw_size > 65535:
        print(f"[ERROR] Firmware size ({fw_size}) exceeds 65535 bytes!")
        return False
        
    fw_crc = crc_slow(fw_bin)
    total_chunks = (fw_size + chunk_size - 1) // chunk_size
    
    print(f"\n[*] OTA Session 0x{session_id:04X}")
    print(f"    File: {os.path.basename(hex_file)}")
    print(f"    Size: {fw_size} bytes ({fw_size/1024:.1f}KB)")
    print(f"    CRC32: 0x{fw_crc:08X}")
    print(f"    Flash: 0x{start_addr:08X} - 0x{end_addr:08X}")
    print(f"    Entry: 0x{entry_point:08X}")
    print(f"    Chunks: {total_chunks} x {chunk_size} bytes")
    print(f"    Include flash addr: {include_flash_addr}")
    
    if interactive:
        confirm = input("\nProceed with OTA? (Y/n): ").strip().lower()
        if confirm == 'n':
            print("OTA cancelled")
            return False
    
    all_responses = []
    success = True
    
    try:
        with serial.Serial(SER_PORT, SER_BAUDRATE, timeout=TIMEOUT) as ser:
            print(f"\n[*] Serial: {SER_PORT} @ {SER_BAUDRATE}")
            time.sleep(0.2)  # Increased stabilization time
            ser.reset_input_buffer()
            ser.reset_output_buffer()
            
            steps = [
                ("START", OTA_TYPE_START, b"\x00", "Begin OTA"),
                ("HEADER", OTA_TYPE_HEADER, pack_header(session_id, fw_size, chunk_size, fw_crc, include_flash_addr, start_addr), 
                 f"session=0x{session_id:04X}, size={fw_size}, chunks={total_chunks}"),
            ]
            
            # Execute START and HEADER
            for step_name, msg_type, payload, info in steps:
                print(f"\n[{msg_type+1}] {step_name}")
                frame = build_frame(msg_type, payload)
                log_data(log_file, "TX", step_name, frame, info)
                ser.write(frame)
                print(f"[TX] {step_name}")
                
                if wait_response:
                    response, got_response = wait_for_response(ser, RESPONSE_TIMEOUT, log_file)
                    if response:
                        all_responses.append((f"{step_name}_RESPONSE", response))
                    elif step_name == "HEADER":  # Header response is critical
                        print(f"[WARNING] No response for {step_name}")
                        if interactive:
                            cont = input("Continue anyway? (y/N): ").strip().lower()
                            if cont != 'y':
                                success = False
                                break
                time.sleep(SLEEP_TIME)
            
            if not success:
                return False
            
            # DATA chunks with progress
            print(f"\n[3] DATA ({total_chunks} chunks)")
            failed_chunks = 0
            for i in range(0, fw_size, chunk_size):
                seq = i // chunk_size
                chunk = fw_bin[i:i+chunk_size]
                
                if include_flash_addr:
                    chunk_addr = start_addr + i
                    payload = struct.pack(">HI", seq, chunk_addr) + chunk
                    addr_info = f" @0x{chunk_addr:08X}"
                else:
                    payload = struct.pack(">H", seq) + chunk
                    addr_info = ""
                
                frame = build_frame(OTA_TYPE_DATA, payload)
                info = f"seq={seq}/{total_chunks-1}, size={len(chunk)}, addr_included={include_flash_addr}"
                log_data(log_file, "TX", "DATA", frame, info)
                ser.write(frame)
                
                progress = (seq + 1) * 100 // total_chunks
                print(f"[TX] DATA {seq:3d}/{total_chunks-1} [{progress:3d}%] {len(chunk):2d}B{addr_info}")
                
                if wait_response:
                    response, got_response = wait_for_response(ser, RESPONSE_TIMEOUT * 0.3, log_file)
                    if response:
                        all_responses.append((f"DATA_RESPONSE_{seq}", response))
                    elif not got_response:
                        failed_chunks += 1
                        if failed_chunks > 5:  # Too many failed chunks
                            print(f"[ERROR] Too many chunks without response ({failed_chunks})")
                            if interactive:
                                cont = input("Continue anyway? (y/N): ").strip().lower()
                                if cont != 'y':
                                    success = False
                                    break
                
                time.sleep(SLEEP_TIME)
            
            if not success:
                return False
            
            # END and SIGNAL_UPDATE
            final_steps = [
                ("END", OTA_TYPE_END, b"\x01", "End data transmission"),
                ("SIGNAL_UPDATE", OTA_TYPE_SIGNAL_UPDATE, struct.pack(">II", start_addr, entry_point), 
                 f"flash=0x{start_addr:08X}, entry=0x{entry_point:08X}")
            ]
            
            for step_name, msg_type, payload, info in final_steps:
                print(f"\n[{msg_type+1}] {step_name}")
                frame = build_frame(msg_type, payload)
                log_data(log_file, "TX", step_name, frame, info)
                ser.write(frame)
                print(f"[TX] {step_name}")
                
                timeout_multiplier = 3 if step_name == "SIGNAL_UPDATE" else 1
                if wait_response:
                    response, got_response = wait_for_response(ser, RESPONSE_TIMEOUT * timeout_multiplier, log_file)
                    if response:
                        all_responses.append((f"{step_name}_RESPONSE", response))
                time.sleep(SLEEP_TIME)
            
    except serial.SerialException as e:
        print(f"[ERROR] Serial error: {e}")
        return False
    except Exception as e:
        print(f"[ERROR] Unexpected error: {e}")
        return False
    
    # Results summary
    print(f"\n{'='*60}")
    print(f"[*] OTA Complete! Session 0x{session_id:04X}")
    print(f"[*] Transmitted: {fw_size} bytes ({fw_size/1024:.1f}KB)")
    print(f"[*] Responses received: {len(all_responses)}")
    if failed_chunks > 0:
        print(f"[*] Failed chunk responses: {failed_chunks}")
    
    # Save response summary
    if all_responses and log_file:
        with open(log_file, 'a', encoding='utf-8') as f:
            f.write(f"\n{'='*70}\n")
            f.write(f"SESSION SUMMARY - 0x{session_id:04X}\n")
            f.write(f"{'='*70}\n")
            f.write(f"File: {hex_file}\n")
            f.write(f"Size: {fw_size} bytes\n")
            f.write(f"CRC: 0x{fw_crc:08X}\n")
            f.write(f"Chunks: {total_chunks}\n")
            f.write(f"Responses: {len(all_responses)}\n")
            f.write(f"Failed chunks: {failed_chunks}\n")
            f.write(f"Status: {'SUCCESS' if success else 'FAILED'}\n\n")
            
            for i, (resp_type, resp_data) in enumerate(all_responses):
                hex_data = ' '.join(f'{b:02X}' for b in resp_data)
                ascii_data = ''.join(chr(b) if 32 <= b < 127 else '.' for b in resp_data)
                f.write(f"{i+1:2d}. {resp_type}\n")
                f.write(f"    HEX:   {hex_data}\n")
                f.write(f"    ASCII: {ascii_data}\n")
                f.write(f"    Size:  {len(resp_data)} bytes\n\n")
        
        print(f"[*] Complete log saved: {log_file}")
    
    return success

# Backward compatibility
send_stm32_ota = send_stm32_ota_enhanced

if __name__ == "__main__":
    print("=== STM32 OTA Server ===")
    
    # Test CRC
    test_crc()
    print(f"\nConfig: Flash=0x{STM32_FLASH_BASE:08X}, Size={STM32_FLASH_SIZE//1024}KB")
    print(f"Serial: {SER_PORT} @ {SER_BAUDRATE}")
    
    # Check command line arguments
    import sys
    if len(sys.argv) > 1:
        if sys.argv[1] == "--interactive" or sys.argv[1] == "-i":
            interactive_menu()
        elif sys.argv[1] == "--help" or sys.argv[1] == "-h":
            print("\nUsage:")
            print("  python script.py                 # Auto-detect firmware and send")
            print("  python script.py -i              # Interactive menu")
            print("  python script.py firmware.hex    # Send specific file")
            print("  python script.py -h              # Show this help")
        else:
            # Send specific file
            firmware_file = sys.argv[1]
            if os.path.exists(firmware_file):
                send_stm32_ota(firmware_file, chunk_size=128, interactive=True)
            else:
                print(f"File not found: {firmware_file}")
    else:
        # Default behavior - auto-detect and send
        send_stm32_ota(chunk_size=128, enable_log=True, include_flash_addr=False, 
                      session_id=0x1234, interactive=False)
