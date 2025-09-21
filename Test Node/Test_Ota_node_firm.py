# import serial
# import time
# import threading
# from datetime import datetime

# # ===============================
# # Hàm build frame OTA (placeholder, bạn thay bằng code thực tế của bạn)
# # ===============================
# def build_frame(frame_type, payload):
#     # Ví dụ đơn giản: [type][len][payload...]
#     frame = bytearray()
#     frame.append(frame_type)
#     frame.append(len(payload))
#     frame.extend(payload)
#     return frame

# # ===============================
# # Logging TX frame
# # ===============================
# def log_frame(log_file, direction, frame_name, frame, desc=""):
#     timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f")[:-3]
#     hex_data = ' '.join(f'{b:02X}' for b in frame)
#     with open(log_file, "a", encoding="utf-8") as f:
#         f.write(f"[{timestamp}] {direction} {frame_name}\n")
#         f.write(f"     HEX: {hex_data}\n")
#         f.write(f"     Size: {len(frame)} bytes\n")
#         if desc:
#             f.write(f"     Desc: {desc}\n")
#         f.write("\n")

# # ===============================
# # Hàm đọc phản hồi STM32
# # ===============================
# def read_and_display_response(ser, log_file=None):
#     try:
#         if ser.in_waiting > 0:
#             data = ser.read(ser.in_waiting)
#             if data:
#                 timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f")[:-3]
#                 hex_data = ' '.join(f'{b:02X}' for b in data)
#                 ascii_data = ''.join(chr(b) if 32 <= b < 127 else '.' for b in data)
                
#                 print(f"[RX] {timestamp} | HEX: {hex_data} | ASCII: {ascii_data}")
                
#                 if log_file:
#                     with open(log_file, "a", encoding="utf-8") as f:
#                         f.write(f"[{timestamp}] RX DATA\n")
#                         f.write(f"     HEX: {hex_data}\n")
#                         f.write(f"     ASCII: {ascii_data}\n")
#                         f.write(f"     Size: {len(data)} bytes\n\n")
#     except Exception as e:
#         print(f"[ERROR] Serial read error: {e}")

# # ===============================
# # Thread chạy nền đọc serial
# # ===============================
# def serial_reader(ser, log_file=None):
#     while True:
#         read_and_display_response(ser, log_file)
#         time.sleep(0.05)  # tránh CPU 100%

# # ===============================
# # Hàm gửi OTA xuống STM32
# # ===============================
# def send_stm32_ota(port="COM3", baudrate=9600, log_file="ota_log.txt"):
#     ser = serial.Serial(port, baudrate, timeout=0.1)

#     # Bật thread đọc RX chạy nền
#     threading.Thread(target=serial_reader, args=(ser, log_file), daemon=True).start()

#     # Gửi START
#     start_payload = b"\x01\x02"   # ví dụ payload
#     frame = build_frame(0x10, start_payload)
#     log_frame(log_file, "TX", "START", frame, "Begin STM32 OTA process")
#     ser.write(frame)
#     print("[TX] START sent")
#     time.sleep(0.5)

#     # Gửi HEADER
#     header_payload = b"\xAA\xBB\xCC\xDD"
#     frame = build_frame(0x11, header_payload)
#     log_frame(log_file, "TX", "HEADER", frame)
#     ser.write(frame)
#     print("[TX] HEADER sent")
#     time.sleep(0.5)

#     # Gửi DATA
#     for i in range(3):  # ví dụ 3 data packet
#         data_payload = bytes([i]) * 8
#         frame = build_frame(0x12, data_payload)
#         log_frame(log_file, "TX", f"DATA[{i}]", frame)
#         ser.write(frame)
#         print(f"[TX] DATA[{i}] sent")
#         time.sleep(0.5)

#     # Gửi END
#     end_payload = b"\xEE"
#     frame = build_frame(0x13, end_payload)
#     log_frame(log_file, "TX", "END", frame)
#     ser.write(frame)
#     print("[TX] END sent")

#     # Giữ cổng mở để nhận phản hồi
#     print("OTA transfer done. Listening for STM32 responses...")
#     try:
#         while True:
#             time.sleep(1)
#     except KeyboardInterrupt:
#         print("Exit by user.")
#         ser.close()

# # ===============================
# # Run thử
# # ===============================
# if __name__ == "__main__":
#     send_stm32_ota("COM9", 9600, "ota_log.txt")












# # import serial
# # import time
# # import struct
# # import os
# # from datetime import datetime

# # # =====================
# # # Cấu hình Serial
# # # =====================
# # SER_PORT = "COM4"  # Thay bằng cổng phù hợp
# # SER_BAUDRATE = 9600
# # TIMEOUT = 0.05
# # SLEEP_TIME = 0.001

# # # =====================
# # # Định nghĩa OTA
# # # =====================
# # SOF = 0xAA
# # EOF = 0xBB
# # PKT_TYPE_OTA = 0x02

# # # message type
# # OTA_TYPE_START         = 0x00
# # OTA_TYPE_HEADER        = 0x01
# # OTA_TYPE_DATA          = 0x02
# # OTA_TYPE_RESPONSE      = 0x03
# # OTA_TYPE_END           = 0x04
# # OTA_TYPE_SIGNAL_UPDATE = 0x05

# # # STM32 Flash Configuration
# # STM32_FLASH_BASE = 0x08000000  # Địa chỉ Flash base của STM32
# # STM32_FLASH_SIZE = 256 * 1024  # 256KB (tùy thuộc vào MCU)
# # STM32_PAGE_SIZE = 2048         # Page size (tùy thuộc vào MCU)

# # # =====================
# # # CRC Custom Implementation
# # # =====================
# # POLYNOMIAL = 0x04C11DB7
# # INITIAL_REMAINDER = 0xFFFFFFFF
# # FINAL_XOR_VALUE = 0xFFFFFFFF
# # WIDTH = 32
# # BITS_PER_BYTE = 8
# # TOPBIT = 0x80000000

# # def reflect(data, n_bits):
# #     """Reflect (reverse) the bits in a data value"""
# #     reflection = 0
# #     for bit in range(n_bits):
# #         if data & (1 << bit):
# #             reflection |= (1 << ((n_bits - 1) - bit))
# #     return reflection

# # def reflect_data(x):
# #     """Reflect 8 bits"""
# #     return reflect(x, BITS_PER_BYTE) & 0xFF

# # def reflect_remainder(x):
# #     """Reflect 32 bits"""
# #     return reflect(x, WIDTH) & 0xFFFFFFFF

# # def crc_slow(p_message):
# #     """Calculate CRC using the slow bit-by-bit method"""
# #     remainder = INITIAL_REMAINDER
    
# #     # Perform modulo-2 division, one byte at a time
# #     for byte_val in p_message:
# #         # Bring the next byte into the remainder
# #         remainder ^= (reflect_data(byte_val) << (WIDTH - BITS_PER_BYTE))
        
# #         # Perform modulo-2 division, one bit at a time
# #         for bit in range(BITS_PER_BYTE, 0, -1):
# #             # Try to divide the current data bit
# #             if remainder & TOPBIT:
# #                 remainder = ((remainder << 1) ^ POLYNOMIAL) & 0xFFFFFFFF
# #             else:
# #                 remainder = (remainder << 1) & 0xFFFFFFFF
    
# #     # The final remainder is the CRC result
# #     return (reflect_remainder(remainder) ^ FINAL_XOR_VALUE) & 0xFFFFFFFF

# # # =====================
# # # Logging functions
# # # =====================
# # def log_frame(log_file, direction, frame_type, frame_data, extra_info=""):
# #     """Log frame data to file"""
# #     timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f")[:-3]
# #     hex_data = ' '.join(f'{b:02X}' for b in frame_data)
    
# #     with open(log_file, 'a', encoding='utf-8') as f:
# #         f.write(f"[{timestamp}] {direction} {frame_type}\n")
# #         if extra_info:
# #             f.write(f"     Info: {extra_info}\n")
# #         f.write(f"     Data: {hex_data}\n")
# #         f.write(f"     Size: {len(frame_data)} bytes\n\n")

# # def init_log_file(log_file):
# #     """Initialize log file with header"""
# #     timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
# #     with open(log_file, 'w', encoding='utf-8') as f:
# #         f.write(f"=== STM32 OTA Server Log - {timestamp} ===\n\n")

# # def pack_header(session_id, fw_size, chunk_size, fw_crc, include_flash_addr=True, flash_addr=STM32_FLASH_BASE):
# #     """
# #     Pack OTA header struct with new format:
# #     - session_id (2B): Random session ID for this OTA transfer
# #     - fw_size (2B): Firmware size in bytes (max 65535)
# #     - chunk_size (2B): Data chunk size in bytes
# #     - total_chunks (2B): Total number of data chunks
# #     - fw_crc (4B): CRC32 of entire firmware
# #     - flash_addr (4B): Optional flash address (if include_flash_addr=True)
    
# #     Total size: 12 bytes (basic) or 16 bytes (with flash_addr)
# #     """
# #     total_chunks = (fw_size + chunk_size - 1) // chunk_size
    
# #     if include_flash_addr:
# #         # 16-byte header with flash address
# #         return struct.pack(">H H H H I I", 
# #                           session_id,    # 2B: Session ID
# #                           fw_size,       # 2B: Firmware size (up to 65535 bytes)
# #                           chunk_size,    # 2B: Chunk size
# #                           total_chunks,  # 2B: Total chunks
# #                           fw_crc,        # 4B: Firmware CRC32
# #                           flash_addr)    # 4B: Flash address
# #     else:
# #         # 12-byte header without flash address (bootloader uses default)
# #         return struct.pack(">H H H H I", 
# #                           session_id,    # 2B: Session ID
# #                           fw_size,       # 2B: Firmware size
# #                           chunk_size,    # 2B: Chunk size
# #                           total_chunks,  # 2B: Total chunks
# #                           fw_crc)        # 4B: Firmware CRC32

# # # =====================
# # # Hàm tạo frame
# # # =====================
# # def build_frame(message_type, payload):
# #     """Build complete OTA frame with custom CRC"""
# #     length = len(payload)
    
# #     # Tạo phần đầu frame (không bao gồm CRC và EOF)
# #     header = struct.pack(">BBB H", SOF, PKT_TYPE_OTA, message_type, length)
    
# #     # Dữ liệu để tính CRC: từ SOF đến hết payload
# #     crc_data = header + payload
    
# #     # Tính CRC theo thuật toán custom
# #     crc_val = crc_slow(crc_data)
    
# #     # Tạo frame hoàn chỉnh
# #     frame = header + payload + struct.pack(">I B", crc_val, EOF)
    
# #     return frame

# # # =====================
# # # Optimized HEX to Binary converter for STM32
# # # =====================
# # def hex_to_stm32_binary(hex_file, target_flash_addr=STM32_FLASH_BASE):
# #     """
# #     Convert Intel HEX file to STM32-optimized binary data
# #     Returns: (binary_data, start_address, end_address, entry_point)
# #     """
# #     print(f"[*] Converting Intel HEX to STM32 binary: {hex_file}")
# #     print(f"[*] Target Flash Address: 0x{target_flash_addr:08X}")
    
# #     # Dictionary to store address -> data mapping
# #     memory_map = {}
# #     extended_address = 0
# #     min_addr = None
# #     max_addr = None
# #     entry_point = None
    
# #     with open(hex_file, "r") as f:
# #         line_num = 0
# #         for line in f:
# #             line_num += 1
# #             line = line.strip()
            
# #             # Skip empty lines or lines not starting with ':'
# #             if not line or not line.startswith(":"):
# #                 continue
            
# #             # Parse Intel HEX record
# #             try:
# #                 # Extract fields
# #                 byte_count = int(line[1:3], 16)
# #                 address = int(line[3:7], 16)
# #                 record_type = int(line[7:9], 16)
# #                 data_hex = line[9:9 + byte_count * 2]
# #                 checksum = int(line[9 + byte_count * 2:11 + byte_count * 2], 16)
                
# #                 # Verify checksum
# #                 calculated_sum = byte_count + (address >> 8) + (address & 0xFF) + record_type
# #                 for i in range(0, len(data_hex), 2):
# #                     calculated_sum += int(data_hex[i:i+2], 16)
# #                 calculated_sum = ((~calculated_sum) + 1) & 0xFF
                
# #                 if calculated_sum != checksum:
# #                     print(f"[WARNING] Checksum error at line {line_num}")
# #                     continue
                
# #                 # Process different record types
# #                 if record_type == 0x00:  # Data Record
# #                     full_address = extended_address + address
# #                     data_bytes = bytes.fromhex(data_hex)
                    
# #                     # Only process data within STM32 Flash range
# #                     if full_address >= target_flash_addr:
# #                         # Store data in memory map
# #                         for i, byte_val in enumerate(data_bytes):
# #                             memory_map[full_address + i] = byte_val
                        
# #                         # Track address range
# #                         if min_addr is None or full_address < min_addr:
# #                             min_addr = full_address
# #                         if max_addr is None or (full_address + len(data_bytes) - 1) > max_addr:
# #                             max_addr = full_address + len(data_bytes) - 1
                            
# #                         print(f"    Data: addr=0x{full_address:08X}, len={len(data_bytes)} bytes")
# #                     else:
# #                         print(f"    Skipped: addr=0x{full_address:08X} (outside target range)")
                    
# #                 elif record_type == 0x01:  # End of File Record
# #                     print(f"    End of file record at line {line_num}")
# #                     break
                    
# #                 elif record_type == 0x02:  # Extended Segment Address Record
# #                     extended_address = int(data_hex, 16) << 4
# #                     print(f"    Extended segment addr: 0x{extended_address:08X}")
                    
# #                 elif record_type == 0x04:  # Extended Linear Address Record  
# #                     extended_address = int(data_hex, 16) << 16
# #                     print(f"    Extended linear addr: 0x{extended_address:08X}")
                    
# #                 elif record_type == 0x05:  # Start Linear Address Record
# #                     entry_point = int(data_hex, 16)
# #                     print(f"    Entry point: 0x{entry_point:08X}")
                    
# #                 else:
# #                     print(f"    Unknown record type {record_type:02X} at line {line_num}")
                    
# #             except (ValueError, IndexError) as e:
# #                 print(f"[ERROR] Invalid HEX format at line {line_num}: {e}")
# #                 continue
    
# #     if not memory_map:
# #         print("[ERROR] No valid data records found for target Flash address!")
# #         return bytes(), 0, 0, 0
    
# #     print(f"[*] Flash Memory range: 0x{min_addr:08X} - 0x{max_addr:08X}")
# #     print(f"[*] Binary data size: {len(memory_map)} bytes")
# #     if entry_point:
# #         print(f"[*] Entry point: 0x{entry_point:08X}")
    
# #     # Convert memory map to continuous binary data
# #     # Start from the minimum address (should be STM32_FLASH_BASE)
# #     bin_data = bytearray()
    
# #     # Create continuous binary from min_addr to max_addr
# #     for addr in range(min_addr, max_addr + 1):
# #         if addr in memory_map:
# #             bin_data.append(memory_map[addr])
# #         else:
# #             bin_data.append(0xFF)  # Fill gaps with 0xFF (erased flash state)
    
# #     # Show memory layout info
# #     print(f"[*] Vector Table (first 32 bytes):")
# #     preview_bytes = min(32, len(bin_data))
# #     for i in range(0, preview_bytes, 4):
# #         if i + 3 < len(bin_data):
# #             word_val = struct.unpack("<I", bin_data[i:i+4])[0]  # Little-endian
# #             if i == 0:
# #                 print(f"    SP (0x{min_addr+i:08X}): 0x{word_val:08X} (Stack Pointer)")
# #             elif i == 4:
# #                 print(f"    PC (0x{min_addr+i:08X}): 0x{word_val:08X} (Reset Vector)")
# #             else:
# #                 print(f"    Vec[{i//4:2d}] (0x{min_addr+i:08X}): 0x{word_val:08X}")
    
# #     # Show first raw bytes
# #     preview_hex = ' '.join(f'{b:02X}' for b in bin_data[:preview_bytes])
# #     print(f"[*] Raw binary (first {preview_bytes} bytes): {preview_hex}")
    
# #     return bytes(bin_data), min_addr, max_addr, entry_point or 0

# # # =====================
# # # Helper function to find firmware file
# # # =====================
# # def find_firmware_file():
# #     """Find the first .hex or .bin file in the current directory."""
# #     script_dir = os.path.dirname(os.path.abspath(__file__))
# #     for ext in ['.hex', '.bin']:
# #         for f in os.listdir(script_dir):
# #             if f.endswith(ext):
# #                 print(f"[*] Auto-detected firmware file: {f}")
# #                 return os.path.join(script_dir, f)
# #     return None

# # # =====================
# # # Enhanced OTA sending function
# # # =====================
# # def send_stm32_ota(hex_file=None, chunk_size=64, enable_log=True, target_flash_addr=STM32_FLASH_BASE, 
# #                    include_flash_addr=True, session_id=None):
# #     """Send STM32 OTA update via serial with new header format"""
    
# #     # Generate random session ID if not provided
# #     if session_id is None:
# #         import random
# #         session_id = random.randint(0x1000, 0xFFFF)
    
# #     # Tự động tìm file firmware nếu không được chỉ định
# #     if hex_file is None:
# #         hex_file = find_firmware_file()
# #         if hex_file is None:
# #             print("[ERROR] No firmware file found in current directory!")
# #             print("        Please ensure you have a .hex or .bin file in the same folder as this script.")
# #             return
    
# #     # Setup log file
# #     log_file = None
# #     if enable_log:
# #         script_dir = os.path.dirname(os.path.abspath(__file__))
# #         log_file = os.path.join(script_dir, "stm32_ota_log.txt")
# #         init_log_file(log_file)
# #         print(f"[*] Logging enabled: {log_file}")
    
# #     print(f"[*] Reading firmware from: {hex_file}")
    
# #     try:
# #         # Convert HEX to STM32 binary
# #         fw_bin, start_addr, end_addr, entry_point = hex_to_stm32_binary(hex_file, target_flash_addr)
        
# #         if not fw_bin:
# #             print("[ERROR] No binary data extracted from HEX file!")
# #             return
            
# #     except FileNotFoundError:
# #         print(f"[ERROR] File not found: {hex_file}")
# #         return
# #     except Exception as e:
# #         print(f"[ERROR] Failed to read hex file: {e}")
# #         return
    
# #     fw_size = len(fw_bin)
# #     if fw_size > 65535:
# #         print(f"[ERROR] Firmware size ({fw_size} bytes) exceeds 65535 bytes limit!")
# #         print("        Consider using uint32_t fw_size or split into multiple segments")
# #         return
        
# #     fw_crc = crc_slow(fw_bin)  # Tính CRC của toàn bộ firmware binary
# #     total_chunks = (fw_size + chunk_size - 1) // chunk_size
    
# #     print(f"\n[*] STM32 OTA Session Information:")
# #     print(f"    Session ID:   0x{session_id:04X}")
# #     print(f"    Binary size:  {fw_size} bytes")
# #     print(f"    Flash start:  0x{start_addr:08X}")
# #     print(f"    Flash end:    0x{end_addr:08X}")
# #     print(f"    Entry point:  0x{entry_point:08X}")
# #     print(f"    Binary CRC:   0x{fw_crc:08X}")
# #     print(f"    Chunk size:   {chunk_size} bytes")
# #     print(f"    Total chunks: {total_chunks}")
# #     print(f"    Include flash addr: {'Yes' if include_flash_addr else 'No'}")
    
# #     try:
# #         with serial.Serial(SER_PORT, SER_BAUDRATE, timeout=TIMEOUT) as ser:
# #             print(f"\n[*] Serial opened on {SER_PORT} @ {SER_BAUDRATE}")
# #             time.sleep(0.1)  # Wait for serial to stabilize
            
# #             # 1. START - Bắt đầu quá trình OTA
# #             start_payload = struct.pack(">H", session_id)  # Include session_id in start
# #             frame = build_frame(OTA_TYPE_START, start_payload)
# #             if enable_log:
# #                 log_frame(log_file, "TX", "START", frame, f"Begin STM32 OTA process, session_id=0x{session_id:04X}")
# #             ser.write(frame)
# #             time.sleep(SLEEP_TIME)
# #             print(f"[TX] START (session_id=0x{session_id:04X})")
            
# #             # 2. HEADER - Gửi thông tin firmware với format mới
# #             header_payload = pack_header(session_id, fw_size, chunk_size, fw_crc, 
# #                                        include_flash_addr, start_addr)
# #             frame = build_frame(OTA_TYPE_HEADER, header_payload)
            
# #             header_info = f"session_id=0x{session_id:04X}, fw_size={fw_size}, chunk_size={chunk_size}, total_chunks={total_chunks}, fw_crc=0x{fw_crc:08X}"
# #             if include_flash_addr:
# #                 header_info += f", flash_addr=0x{start_addr:08X}"
                
# #             if enable_log:
# #                 log_frame(log_file, "TX", "HEADER", frame, header_info)
                
# #             ser.write(frame)
# #             time.sleep(SLEEP_TIME)
            
# #             if include_flash_addr:
# #                 print(f"[TX] HEADER (session=0x{session_id:04X}, fw_size={fw_size}, chunks={total_chunks}, flash=0x{start_addr:08X})")
# #             else:
# #                 print(f"[TX] HEADER (session=0x{session_id:04X}, fw_size={fw_size}, chunks={total_chunks})")
            
# #             # 3. DATA - Gửi raw binary data theo chunks
# #             seq = 0
            
# #             print(f"\n[*] Sending {total_chunks} data chunks...")
# #             for i in range(0, fw_size, chunk_size):
# #                 chunk = fw_bin[i:i+chunk_size]
# #                 payload = struct.pack(">H H", session_id, seq) + chunk  # Session ID + Sequence + data
# #                 frame = build_frame(OTA_TYPE_DATA, payload)
                
# #                 if enable_log:
# #                     chunk_addr = start_addr + i
# #                     log_frame(log_file, "TX", "DATA", frame, 
# #                              f"session=0x{session_id:04X}, seq={seq}/{total_chunks-1}, size={len(chunk)} bytes, flash_addr=0x{chunk_addr:08X}")
                
# #                 ser.write(frame)
# #                 time.sleep(SLEEP_TIME)
                
# #                 # Progress indicator
# #                 progress = (seq + 1) * 100 // total_chunks
# #                 print(f"[TX] DATA session=0x{session_id:04X} seq={seq:3d}/{total_chunks-1} [{progress:3d}%] size={len(chunk):2d} bytes @ 0x{start_addr + i:08X}")
# #                 seq += 1
            
# #             # 4. END - Kết thúc truyền data
# #             end_payload = struct.pack(">H B", session_id, 0x01)  # Session ID + End flag
# #             frame = build_frame(OTA_TYPE_END, end_payload)
# #             if enable_log:
# #                 log_frame(log_file, "TX", "END", frame, f"End of firmware binary data, session=0x{session_id:04X}")
# #             ser.write(frame)
# #             time.sleep(SLEEP_TIME)
# #             print(f"[TX] END (session=0x{session_id:04X})")
            
# #             # 5. SIGNAL_UPDATE - Yêu cầu STM32 thực hiện update
# #             update_info = struct.pack(">H I I", session_id, start_addr, entry_point)  # Session + Flash addr + Entry point
# #             frame = build_frame(OTA_TYPE_SIGNAL_UPDATE, update_info)
# #             if enable_log:
# #                 log_frame(log_file, "TX", "SIGNAL_UPDATE", frame, 
# #                          f"session=0x{session_id:04X}, flash_addr=0x{start_addr:08X}, entry_point=0x{entry_point:08X}")
# #             ser.write(frame)
# #             time.sleep(SLEEP_TIME)
# #             print(f"[TX] SIGNAL_UPDATE (session=0x{session_id:04X}, flash=0x{start_addr:08X}, entry=0x{entry_point:08X})")
            
# #     except serial.SerialException as e:
# #         print(f"[ERROR] Serial error: {e}")
# #         return
# #     except Exception as e:
# #         print(f"[ERROR] Unexpected error: {e}")
# #         return
    
# #     print(f"\n[*] STM32 OTA transmission completed successfully!")
# #     print(f"[*] Session 0x{session_id:04X}: {fw_size} bytes of raw binary data sent to STM32 Flash")
# #     if enable_log:
# #         print(f"[*] Check log file for detailed frame data: {log_file}")

# # # =====================
# # # Test CRC function
# # # =====================
# # def test_crc():
# #     """Test CRC calculation with known data"""
# #     test_data = b"123456789"
# #     crc_result = crc_slow(test_data)
# #     print(f"CRC of '{test_data.decode()}': 0x{crc_result:08X}")
    
# #     # Test với STM32 vector table
# #     stm32_vectors = bytes([0x00, 0x20, 0x00, 0x20, 0xED, 0x00, 0x00, 0x08])  # Typical STM32 start
# #     crc_vectors = crc_slow(stm32_vectors)
# #     print(f"CRC of STM32 vectors: 0x{crc_vectors:08X}")

# # # =====================
# # # Main
# # # =====================
# # if __name__ == "__main__":
# #     print("=== STM32 OTA Server với Raw Binary Data ===")
    
# #     # Test CRC trước
# #     print("\n[*] Testing CRC calculation:")
# #     test_crc()
    
# #     print(f"\n[*] STM32 Configuration:")
# #     print(f"    Flash Base: 0x{STM32_FLASH_BASE:08X}")
# #     print(f"    Flash Size: {STM32_FLASH_SIZE} bytes ({STM32_FLASH_SIZE//1024}KB)")
# #     print(f"    Page Size:  {STM32_PAGE_SIZE} bytes")
    
# #     print(f"\n[*] Serial Configuration:")
# #     print(f"    Port: {SER_PORT}")
# #     print(f"    Baudrate: {SER_BAUDRATE}")
    
# #     # Gửi STM32 OTA với tối ưu hóa và session ID
# #     print("\n=== Example Usage ===")
# #     print("# With flash address (16-byte header):")
# #     print('send_stm32_ota("firmware.hex", chunk_size=64, include_flash_addr=True, session_id=0x1234)')
# #     print("\n# Without flash address (12-byte header - bootloader uses default):")
# #     print('send_stm32_ota("firmware.hex", chunk_size=64, include_flash_addr=False)')
# #     print("\n# Auto-generate session ID:")
# #     print('send_stm32_ota("firmware.hex", chunk_size=64)')
    
# #     # Example call
# #     send_stm32_ota("firmware.hex", chunk_size=128, enable_log=True, include_flash_addr=False, session_id=0x1234)

# import serial
# import time
# import struct
# import os
# from datetime import datetime

# # =====================
# # Cấu hình Serial
# # =====================
# SER_PORT = "COM4"  # Thay bằng cổng phù hợp
# SER_BAUDRATE = 9600
# TIMEOUT = 0.05
# SLEEP_TIME = 0.001

# # =====================
# # Định nghĩa OTA
# # =====================
# SOF = 0xAA
# EOF = 0xBB
# PKT_TYPE_OTA = 0x02

# # message type
# OTA_TYPE_START         = 0x00
# OTA_TYPE_HEADER        = 0x01
# OTA_TYPE_DATA          = 0x02
# OTA_TYPE_RESPONSE      = 0x03
# OTA_TYPE_END           = 0x04
# OTA_TYPE_SIGNAL_UPDATE = 0x05

# # STM32 Flash Configuration
# STM32_FLASH_BASE = 0x08000000  # Địa chỉ Flash base của STM32
# STM32_FLASH_SIZE = 256 * 1024  # 256KB (tùy thuộc vào MCU)
# STM32_PAGE_SIZE = 2048         # Page size (tùy thuộc vào MCU)

# # =====================
# # CRC Custom Implementation
# # =====================
# POLYNOMIAL = 0x04C11DB7
# INITIAL_REMAINDER = 0xFFFFFFFF
# FINAL_XOR_VALUE = 0xFFFFFFFF
# WIDTH = 32
# BITS_PER_BYTE = 8
# TOPBIT = 0x80000000

# def reflect(data, n_bits):
#     """Reflect (reverse) the bits in a data value"""
#     reflection = 0
#     for bit in range(n_bits):
#         if data & (1 << bit):
#             reflection |= (1 << ((n_bits - 1) - bit))
#     return reflection

# def reflect_data(x):
#     """Reflect 8 bits"""
#     return reflect(x, BITS_PER_BYTE) & 0xFF

# def reflect_remainder(x):
#     """Reflect 32 bits"""
#     return reflect(x, WIDTH) & 0xFFFFFFFF

# def crc_slow(p_message):
#     """Calculate CRC using the slow bit-by-bit method"""
#     remainder = INITIAL_REMAINDER
    
#     # Perform modulo-2 division, one byte at a time
#     for byte_val in p_message:
#         # Bring the next byte into the remainder
#         remainder ^= (reflect_data(byte_val) << (WIDTH - BITS_PER_BYTE))
        
#         # Perform modulo-2 division, one bit at a time
#         for bit in range(BITS_PER_BYTE, 0, -1):
#             # Try to divide the current data bit
#             if remainder & TOPBIT:
#                 remainder = ((remainder << 1) ^ POLYNOMIAL) & 0xFFFFFFFF
#             else:
#                 remainder = (remainder << 1) & 0xFFFFFFFF
    
#     # The final remainder is the CRC result
#     return (reflect_remainder(remainder) ^ FINAL_XOR_VALUE) & 0xFFFFFFFF

# # =====================
# # Logging functions
# # =====================
# def log_frame(log_file, direction, frame_type, frame_data, extra_info=""):
#     """Log frame data to file"""
#     timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f")[:-3]
#     hex_data = ' '.join(f'{b:02X}' for b in frame_data)
    
#     with open(log_file, 'a', encoding='utf-8') as f:
#         f.write(f"[{timestamp}] {direction} {frame_type}\n")
#         if extra_info:
#             f.write(f"     Info: {extra_info}\n")
#         f.write(f"     Data: {hex_data}\n")
#         f.write(f"     Size: {len(frame_data)} bytes\n\n")

# def init_log_file(log_file):
#     """Initialize log file with header"""
#     timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
#     with open(log_file, 'w', encoding='utf-8') as f:
#         f.write(f"=== STM32 OTA Server Log - {timestamp} ===\n\n")

# def pack_header(session_id, fw_size, chunk_size, fw_crc, include_flash_addr=True, flash_addr=STM32_FLASH_BASE):
#     """
#     Pack OTA header struct with new format:
#     - session_id (2B): Random session ID for this OTA transfer
#     - fw_size (2B): Firmware size in bytes (max 65535)
#     - chunk_size (2B): Data chunk size in bytes
#     - total_chunks (2B): Total number of data chunks
#     - fw_crc (4B): CRC32 of entire firmware
#     - flash_addr (4B): Optional flash address (if include_flash_addr=True)
    
#     Total size: 12 bytes (basic) or 16 bytes (with flash_addr)
#     """
#     total_chunks = (fw_size + chunk_size - 1) // chunk_size
    
#     if include_flash_addr:
#         # 16-byte header with flash address
#         return struct.pack(">H H H H I I", 
#                           session_id,    # 2B: Session ID
#                           fw_size,       # 2B: Firmware size (up to 65535 bytes)
#                           chunk_size,    # 2B: Chunk size
#                           total_chunks,  # 2B: Total chunks
#                           fw_crc,        # 4B: Firmware CRC32
#                           flash_addr)    # 4B: Flash address
#     else:
#         # 12-byte header without flash address (bootloader uses default)
#         return struct.pack(">H H H H I", 
#                           session_id,    # 2B: Session ID
#                           fw_size,       # 2B: Firmware size
#                           chunk_size,    # 2B: Chunk size
#                           total_chunks,  # 2B: Total chunks
#                           fw_crc)        # 4B: Firmware CRC32

# # =====================
# # Hàm tạo frame
# # =====================
# def build_frame(message_type, payload):
#     """Build complete OTA frame with custom CRC"""
#     length = len(payload)
    
#     # Tạo phần đầu frame (không bao gồm CRC và EOF)
#     header = struct.pack(">BBB H", SOF, PKT_TYPE_OTA, message_type, length)
    
#     # Dữ liệu để tính CRC: từ SOF đến hết payload
#     crc_data = header + payload
    
#     # Tính CRC theo thuật toán custom
#     crc_val = crc_slow(crc_data)
    
#     # Tạo frame hoàn chỉnh
#     frame = header + payload + struct.pack(">I B", crc_val, EOF)
    
#     return frame

# # =====================
# # Optimized HEX to Binary converter for STM32
# # =====================
# def hex_to_stm32_binary(hex_file, target_flash_addr=STM32_FLASH_BASE):
#     """
#     Convert Intel HEX file to STM32-optimized binary data
#     Returns: (binary_data, start_address, end_address, entry_point)
#     """
#     print(f"[*] Converting Intel HEX to STM32 binary: {hex_file}")
#     print(f"[*] Target Flash Address: 0x{target_flash_addr:08X}")
    
#     # Dictionary to store address -> data mapping
#     memory_map = {}
#     extended_address = 0
#     min_addr = None
#     max_addr = None
#     entry_point = None
    
#     with open(hex_file, "r") as f:
#         line_num = 0
#         for line in f:
#             line_num += 1
#             line = line.strip()
            
#             # Skip empty lines or lines not starting with ':'
#             if not line or not line.startswith(":"):
#                 continue
            
#             # Parse Intel HEX record
#             try:
#                 # Extract fields
#                 byte_count = int(line[1:3], 16)
#                 address = int(line[3:7], 16)
#                 record_type = int(line[7:9], 16)
#                 data_hex = line[9:9 + byte_count * 2]
#                 checksum = int(line[9 + byte_count * 2:11 + byte_count * 2], 16)
                
#                 # Verify checksum
#                 calculated_sum = byte_count + (address >> 8) + (address & 0xFF) + record_type
#                 for i in range(0, len(data_hex), 2):
#                     calculated_sum += int(data_hex[i:i+2], 16)
#                 calculated_sum = ((~calculated_sum) + 1) & 0xFF
                
#                 if calculated_sum != checksum:
#                     print(f"[WARNING] Checksum error at line {line_num}")
#                     continue
                
#                 # Process different record types
#                 if record_type == 0x00:  # Data Record
#                     full_address = extended_address + address
#                     data_bytes = bytes.fromhex(data_hex)
                    
#                     # Only process data within STM32 Flash range
#                     if full_address >= target_flash_addr:
#                         # Store data in memory map
#                         for i, byte_val in enumerate(data_bytes):
#                             memory_map[full_address + i] = byte_val
                        
#                         # Track address range
#                         if min_addr is None or full_address < min_addr:
#                             min_addr = full_address
#                         if max_addr is None or (full_address + len(data_bytes) - 1) > max_addr:
#                             max_addr = full_address + len(data_bytes) - 1
                            
#                         print(f"    Data: addr=0x{full_address:08X}, len={len(data_bytes)} bytes")
#                     else:
#                         print(f"    Skipped: addr=0x{full_address:08X} (outside target range)")
                    
#                 elif record_type == 0x01:  # End of File Record
#                     print(f"    End of file record at line {line_num}")
#                     break
                    
#                 elif record_type == 0x02:  # Extended Segment Address Record
#                     extended_address = int(data_hex, 16) << 4
#                     print(f"    Extended segment addr: 0x{extended_address:08X}")
                    
#                 elif record_type == 0x04:  # Extended Linear Address Record  
#                     extended_address = int(data_hex, 16) << 16
#                     print(f"    Extended linear addr: 0x{extended_address:08X}")
                    
#                 elif record_type == 0x05:  # Start Linear Address Record
#                     entry_point = int(data_hex, 16)
#                     print(f"    Entry point: 0x{entry_point:08X}")
                    
#                 else:
#                     print(f"    Unknown record type {record_type:02X} at line {line_num}")
                    
#             except (ValueError, IndexError) as e:
#                 print(f"[ERROR] Invalid HEX format at line {line_num}: {e}")
#                 continue
    
#     if not memory_map:
#         print("[ERROR] No valid data records found for target Flash address!")
#         return bytes(), 0, 0, 0
    
#     print(f"[*] Flash Memory range: 0x{min_addr:08X} - 0x{max_addr:08X}")
#     print(f"[*] Binary data size: {len(memory_map)} bytes")
#     if entry_point:
#         print(f"[*] Entry point: 0x{entry_point:08X}")
    
#     # Convert memory map to continuous binary data
#     # Start from the minimum address (should be STM32_FLASH_BASE)
#     bin_data = bytearray()
    
#     # Create continuous binary from min_addr to max_addr
#     for addr in range(min_addr, max_addr + 1):
#         if addr in memory_map:
#             bin_data.append(memory_map[addr])
#         else:
#             bin_data.append(0xFF)  # Fill gaps with 0xFF (erased flash state)
    
#     # Show memory layout info
#     print(f"[*] Vector Table (first 32 bytes):")
#     preview_bytes = min(32, len(bin_data))
#     for i in range(0, preview_bytes, 4):
#         if i + 3 < len(bin_data):
#             word_val = struct.unpack("<I", bin_data[i:i+4])[0]  # Little-endian
#             if i == 0:
#                 print(f"    SP (0x{min_addr+i:08X}): 0x{word_val:08X} (Stack Pointer)")
#             elif i == 4:
#                 print(f"    PC (0x{min_addr+i:08X}): 0x{word_val:08X} (Reset Vector)")
#             else:
#                 print(f"    Vec[{i//4:2d}] (0x{min_addr+i:08X}): 0x{word_val:08X}")
    
#     # Show first raw bytes
#     preview_hex = ' '.join(f'{b:02X}' for b in bin_data[:preview_bytes])
#     print(f"[*] Raw binary (first {preview_bytes} bytes): {preview_hex}")
    
#     return bytes(bin_data), min_addr, max_addr, entry_point or 0

# # =====================
# # Helper function to find firmware file
# # =====================
# def find_firmware_file():
#     """Find the first .hex or .bin file in the current directory."""
#     script_dir = os.path.dirname(os.path.abspath(__file__))
#     for ext in ['.hex', '.bin']:
#         for f in os.listdir(script_dir):
#             if f.endswith(ext):
#                 print(f"[*] Auto-detected firmware file: {f}")
#                 return os.path.join(script_dir, f)
#     return None

# # =====================
# # Enhanced OTA sending function
# # =====================
# def send_stm32_ota(hex_file=None, chunk_size=64, enable_log=True, target_flash_addr=STM32_FLASH_BASE, 
#                    include_flash_addr=True, session_id=None):
#     """Send STM32 OTA update via serial with new header format"""
    
#     # Generate random session ID if not provided
#     if session_id is None:
#         import random
#         session_id = random.randint(0x1000, 0xFFFF)
    
#     # Tự động tìm file firmware nếu không được chỉ định
#     if hex_file is None:
#         hex_file = find_firmware_file()
#         if hex_file is None:
#             print("[ERROR] No firmware file found in current directory!")
#             print("        Please ensure you have a .hex or .bin file in the same folder as this script.")
#             return
    
#     # Setup log file
#     log_file = None
#     if enable_log:
#         script_dir = os.path.dirname(os.path.abspath(__file__))
#         log_file = os.path.join(script_dir, "stm32_ota_log.txt")
#         init_log_file(log_file)
#         print(f"[*] Logging enabled: {log_file}")
    
#     print(f"[*] Reading firmware from: {hex_file}")
    
#     try:
#         # Convert HEX to STM32 binary
#         fw_bin, start_addr, end_addr, entry_point = hex_to_stm32_binary(hex_file, target_flash_addr)
        
#         if not fw_bin:
#             print("[ERROR] No binary data extracted from HEX file!")
#             return
            
#     except FileNotFoundError:
#         print(f"[ERROR] File not found: {hex_file}")
#         return
#     except Exception as e:
#         print(f"[ERROR] Failed to read hex file: {e}")
#         return
    
#     fw_size = len(fw_bin)
#     if fw_size > 65535:
#         print(f"[ERROR] Firmware size ({fw_size} bytes) exceeds 65535 bytes limit!")
#         print("        Consider using uint32_t fw_size or split into multiple segments")
#         return
        
#     fw_crc = crc_slow(fw_bin)  # Tính CRC của toàn bộ firmware binary
#     total_chunks = (fw_size + chunk_size - 1) // chunk_size
    
#     print(f"\n[*] STM32 OTA Session Information:")
#     print(f"    Session ID:   0x{session_id:04X}")
#     print(f"    Binary size:  {fw_size} bytes")
#     print(f"    Flash start:  0x{start_addr:08X}")
#     print(f"    Flash end:    0x{end_addr:08X}")
#     print(f"    Entry point:  0x{entry_point:08X}")
#     print(f"    Binary CRC:   0x{fw_crc:08X}")
#     print(f"    Chunk size:   {chunk_size} bytes")
#     print(f"    Total chunks: {total_chunks}")
#     print(f"    Include flash addr: {'Yes' if include_flash_addr else 'No'}")
    
#     try:
#         with serial.Serial(SER_PORT, SER_BAUDRATE, timeout=TIMEOUT) as ser:
#             print(f"\n[*] Serial opened on {SER_PORT} @ {SER_BAUDRATE}")
#             time.sleep(0.1)  # Wait for serial to stabilize
            
#             # 1. START - Bắt đầu quá trình OTA
#             start_payload = b"\x00"  # Payload: 0x00 (1 byte)
#             frame = build_frame(OTA_TYPE_START, start_payload)
#             if enable_log:
#                 log_frame(log_file, "TX", "START", frame, "Begin STM32 OTA process")
#             ser.write(frame)
#             time.sleep(SLEEP_TIME)
#             print("[TX] START")
            
#             # 2. HEADER - Gửi thông tin firmware với session_id
#             header_payload = pack_header(session_id, fw_size, chunk_size, fw_crc, 
#                                        include_flash_addr, start_addr)
#             frame = build_frame(OTA_TYPE_HEADER, header_payload)
            
#             header_info = f"session_id=0x{session_id:04X}, fw_size={fw_size}, chunk_size={chunk_size}, total_chunks={total_chunks}, fw_crc=0x{fw_crc:08X}"
#             if include_flash_addr:
#                 header_info += f", flash_addr=0x{start_addr:08X}"
                
#             if enable_log:
#                 log_frame(log_file, "TX", "HEADER", frame, header_info)
                
#             ser.write(frame)
#             time.sleep(SLEEP_TIME)
            
#             if include_flash_addr:
#                 print(f"[TX] HEADER (session=0x{session_id:04X}, fw_size={fw_size}, chunks={total_chunks}, flash=0x{start_addr:08X})")
#             else:
#                 print(f"[TX] HEADER (session=0x{session_id:04X}, fw_size={fw_size}, chunks={total_chunks})")
            
#             # 3. DATA - Gửi raw binary data theo chunks
#             seq = 0
            
#             print(f"\n[*] Sending {total_chunks} data chunks...")
#             for i in range(0, fw_size, chunk_size):
#                 chunk = fw_bin[i:i+chunk_size]
#                 # Payload: SEQ (2 bytes) + [Flash Address (4 bytes, optional)] + Raw Binary Data
#                 if include_flash_addr:
#                     chunk_addr = start_addr + i
#                     payload = struct.pack(">H I", seq, chunk_addr) + chunk
#                 else:
#                     payload = struct.pack(">H", seq) + chunk
#                 frame = build_frame(OTA_TYPE_DATA, payload)
                
#                 if enable_log:
#                     chunk_addr = start_addr + i
#                     extra_info = f"seq={seq}/{total_chunks-1}, size={len(chunk)} bytes"
#                     if include_flash_addr:
#                         extra_info += f", flash_addr=0x{chunk_addr:08X}"
#                     log_frame(log_file, "TX", "DATA", frame, extra_info)
                
#                 ser.write(frame)
#                 time.sleep(SLEEP_TIME)
                
#                 # Progress indicator
#                 progress = (seq + 1) * 100 // total_chunks
#                 extra_info = f"[TX] DATA seq={seq:3d}/{total_chunks-1} [{progress:3d}%] size={len(chunk):2d} bytes"
#                 if include_flash_addr:
#                     extra_info += f" @ 0x{chunk_addr:08X}"
#                 print(extra_info)
#                 seq += 1
            
#             # 4. END - Kết thúc truyền data
#             end_payload = b"\x01"  # Payload: 0x01 (1 byte)
#             frame = build_frame(OTA_TYPE_END, end_payload)
#             if enable_log:
#                 log_frame(log_file, "TX", "END", frame, "End of firmware binary data")
#             ser.write(frame)
#             time.sleep(SLEEP_TIME)
#             print("[TX] END")
            
#             # 5. SIGNAL_UPDATE - Yêu cầu STM32 thực hiện update
#             update_info = struct.pack(">I I", start_addr, entry_point)  # Flash addr + Entry point
#             frame = build_frame(OTA_TYPE_SIGNAL_UPDATE, update_info)
#             if enable_log:
#                 log_frame(log_file, "TX", "SIGNAL_UPDATE", frame, 
#                          f"flash_addr=0x{start_addr:08X}, entry_point=0x{entry_point:08X}")
#             ser.write(frame)
#             time.sleep(SLEEP_TIME)
#             print(f"[TX] SIGNAL_UPDATE (flash_addr=0x{start_addr:08X}, entry=0x{entry_point:08X})")
            
#     except serial.SerialException as e:
#         print(f"[ERROR] Serial error: {e}")
#         return
#     except Exception as e:
#         print(f"[ERROR] Unexpected error: {e}")
#         return
    
#     print(f"\n[*] STM32 OTA transmission completed successfully!")
#     print(f"[*] Session 0x{session_id:04X}: {fw_size} bytes of raw binary data sent to STM32 Flash")
#     if enable_log:
#         print(f"[*] Check log file for detailed frame data: {log_file}")

# # =====================
# # Test CRC function
# # =====================
# def test_crc():
#     """Test CRC calculation with known data"""
#     test_data = b"123456789"
#     crc_result = crc_slow(test_data)
#     print(f"CRC of '{test_data.decode()}': 0x{crc_result:08X}")
    
#     # Test với STM32 vector table
#     stm32_vectors = bytes([0x00, 0x20, 0x00, 0x20, 0xED, 0x00, 0x00, 0x08])  # Typical STM32 start
#     crc_vectors = crc_slow(stm32_vectors)
#     print(f"CRC of STM32 vectors: 0x{crc_vectors:08X}")

# # =====================
# # Main
# # =====================
# if __name__ == "__main__":
#     print("=== STM32 OTA Server với Raw Binary Data ===")
    
#     # Test CRC trước
#     print("\n[*] Testing CRC calculation:")
#     test_crc()
    
#     print(f"\n[*] STM32 Configuration:")
#     print(f"    Flash Base: 0x{STM32_FLASH_BASE:08X}")
#     print(f"    Flash Size: {STM32_FLASH_SIZE} bytes ({STM32_FLASH_SIZE//1024}KB)")
#     print(f"    Page Size:  {STM32_PAGE_SIZE} bytes")
    
#     print(f"\n[*] Serial Configuration:")
#     print(f"    Port: {SER_PORT}")
#     print(f"    Baudrate: {SER_BAUDRATE}")
    
#     # Gửi STM32 OTA với tối ưu hóa và session ID
#     print("\n=== Example Usage ===")
#     print("# With flash address (16-byte header):")
#     print('send_stm32_ota("firmware.hex", chunk_size=64, include_flash_addr=True, session_id=0x1234)')
#     print("\n# Without flash address (12-byte header - bootloader uses default):")
#     print('send_stm32_ota("firmware.hex", chunk_size=64, include_flash_addr=False)')
#     print("\n# Auto-generate session ID:")
#     print('send_stm32_ota("firmware.hex", chunk_size=64)')
    
#     # Example call
#     send_stm32_ota("firmware.hex", chunk_size=128, enable_log=True, include_flash_addr=False, session_id=0x1234)

import serial
import time
import struct
import os
from datetime import datetime

# =====================
# Cấu hình Serial
# =====================
SER_PORT = "COM15"  # Thay bằng cổng phù hợp
SER_BAUDRATE = 9600
TIMEOUT = 0.05
SLEEP_TIME = 0.001

# =====================
# Định nghĩa OTA
# =====================
SOF = 0xAA
EOF = 0xBB
PKT_TYPE_OTA = 0x02

# message type
OTA_TYPE_START         = 0x00
OTA_TYPE_HEADER        = 0x01
OTA_TYPE_DATA          = 0x02
OTA_TYPE_RESPONSE      = 0x03
OTA_TYPE_END           = 0x04
OTA_TYPE_SIGNAL_UPDATE = 0x05

# STM32 Flash Configuration
STM32_FLASH_BASE = 0x08000000  # Địa chỉ Flash base của STM32
STM32_FLASH_SIZE = 256 * 1024  # 256KB (tùy thuộc vào MCU)
STM32_PAGE_SIZE = 2048         # Page size (tùy thuộc vào MCU)

# =====================
# CRC Custom Implementation
# =====================
POLYNOMIAL = 0x04C11DB7
INITIAL_REMAINDER = 0xFFFFFFFF
FINAL_XOR_VALUE = 0xFFFFFFFF
WIDTH = 32
BITS_PER_BYTE = 8
TOPBIT = 0x80000000

def reflect(data, n_bits):
    """Reflect (reverse) the bits in a data value"""
    reflection = 0
    for bit in range(n_bits):
        if data & (1 << bit):
            reflection |= (1 << ((n_bits - 1) - bit))
    return reflection

def reflect_data(x):
    """Reflect 8 bits"""
    return reflect(x, BITS_PER_BYTE) & 0xFF

def reflect_remainder(x):
    """Reflect 32 bits"""
    return reflect(x, WIDTH) & 0xFFFFFFFF

def crc_slow(p_message):
    """Calculate CRC using the slow bit-by-bit method"""
    remainder = INITIAL_REMAINDER
    
    # Perform modulo-2 division, one byte at a time
    for byte_val in p_message:
        # Bring the next byte into the remainder
        remainder ^= (reflect_data(byte_val) << (WIDTH - BITS_PER_BYTE))
        
        # Perform modulo-2 division, one bit at a time
        for bit in range(BITS_PER_BYTE, 0, -1):
            # Try to divide the current data bit
            if remainder & TOPBIT:
                remainder = ((remainder << 1) ^ POLYNOMIAL) & 0xFFFFFFFF
            else:
                remainder = (remainder << 1) & 0xFFFFFFFF
    
    # The final remainder is the CRC result
    return (reflect_remainder(remainder) ^ FINAL_XOR_VALUE) & 0xFFFFFFFF

# =====================
# Logging functions
# =====================
def log_frame(log_file, direction, frame_type, frame_data, extra_info=""):
    """Log frame data to file"""
    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f")[:-3]
    hex_data = ' '.join(f'{b:02X}' for b in frame_data)
    
    with open(log_file, 'a', encoding='utf-8') as f:
        f.write(f"[{timestamp}] {direction} {frame_type}\n")
        if extra_info:
            f.write(f"     Info: {extra_info}\n")
        f.write(f"     Data: {hex_data}\n")
        f.write(f"     Size: {len(frame_data)} bytes\n\n")

def init_log_file(log_file):
    """Initialize log file with header"""
    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    with open(log_file, 'w', encoding='utf-8') as f:
        f.write(f"=== STM32 OTA Server Log - {timestamp} ===\n\n")

def pack_header(session_id, fw_size, chunk_size, fw_crc, include_flash_addr=True, flash_addr=STM32_FLASH_BASE):
    """
    Pack OTA header struct with new format:
    - session_id (2B): Random session ID for this OTA transfer
    - fw_size (2B): Firmware size in bytes (max 65535)
    - chunk_size (2B): Data chunk size in bytes
    - total_chunks (2B): Total number of data chunks
    - fw_crc (4B): CRC32 of entire firmware
    - flash_addr (4B): Optional flash address (if include_flash_addr=True)
    
    Total size: 12 bytes (basic) or 16 bytes (with flash_addr)
    """
    total_chunks = (fw_size + chunk_size - 1) // chunk_size
    
    if include_flash_addr:
        # 16-byte header with flash address
        return struct.pack(">H H H H I I", 
                          session_id,    # 2B: Session ID
                          fw_size,       # 2B: Firmware size (up to 65535 bytes)
                          chunk_size,    # 2B: Chunk size
                          total_chunks,  # 2B: Total chunks
                          fw_crc,        # 4B: Firmware CRC32
                          flash_addr)    # 4B: Flash address
    else:
        # 12-byte header without flash address (bootloader uses default)
        return struct.pack(">H H H H I", 
                          session_id,    # 2B: Session ID
                          fw_size,       # 2B: Firmware size
                          chunk_size,    # 2B: Chunk size
                          total_chunks,  # 2B: Total chunks
                          fw_crc)        # 4B: Firmware CRC32

# =====================
# Hàm tạo frame
# =====================
def build_frame(message_type, payload):
    """Build complete OTA frame with custom CRC"""
    length = len(payload)
    
    # Tạo phần đầu frame (không bao gồm CRC và EOF)
    header = struct.pack(">BBB H", SOF, PKT_TYPE_OTA, message_type, length)
    
    # Dữ liệu để tính CRC: từ SOF đến hết payload
    crc_data = header + payload
    
    # Tính CRC theo thuật toán custom
    crc_val = crc_slow(crc_data)
    
    # Tạo frame hoàn chỉnh
    frame = header + payload + struct.pack(">I B", crc_val, EOF)
    
    return frame

# =====================
# Optimized HEX to Binary converter for STM32
# =====================
def hex_to_stm32_binary(hex_file, target_flash_addr=STM32_FLASH_BASE):
    """
    Convert Intel HEX file to STM32-optimized binary data
    Returns: (binary_data, start_address, end_address, entry_point)
    """
    print(f"[*] Converting Intel HEX to STM32 binary: {hex_file}")
    print(f"[*] Target Flash Address: 0x{target_flash_addr:08X}")
    
    # Dictionary to store address -> data mapping
    memory_map = {}
    extended_address = 0
    min_addr = None
    max_addr = None
    entry_point = None
    
    with open(hex_file, "r") as f:
        line_num = 0
        for line in f:
            line_num += 1
            line = line.strip()
            
            # Skip empty lines or lines not starting with ':'
            if not line or not line.startswith(":"):
                continue
            
            # Parse Intel HEX record
            try:
                # Extract fields
                byte_count = int(line[1:3], 16)
                address = int(line[3:7], 16)
                record_type = int(line[7:9], 16)
                data_hex = line[9:9 + byte_count * 2]
                checksum = int(line[9 + byte_count * 2:11 + byte_count * 2], 16)
                
                # Verify checksum
                calculated_sum = byte_count + (address >> 8) + (address & 0xFF) + record_type
                for i in range(0, len(data_hex), 2):
                    calculated_sum += int(data_hex[i:i+2], 16)
                calculated_sum = ((~calculated_sum) + 1) & 0xFF
                
                if calculated_sum != checksum:
                    print(f"[WARNING] Checksum error at line {line_num}")
                    continue
                
                # Process different record types
                if record_type == 0x00:  # Data Record
                    full_address = extended_address + address
                    data_bytes = bytes.fromhex(data_hex)
                    
                    # Only process data within STM32 Flash range
                    if full_address >= target_flash_addr:
                        # Store data in memory map
                        for i, byte_val in enumerate(data_bytes):
                            memory_map[full_address + i] = byte_val
                        
                        # Track address range
                        if min_addr is None or full_address < min_addr:
                            min_addr = full_address
                        if max_addr is None or (full_address + len(data_bytes) - 1) > max_addr:
                            max_addr = full_address + len(data_bytes) - 1
                            
                        print(f"    Data: addr=0x{full_address:08X}, len={len(data_bytes)} bytes")
                    else:
                        print(f"    Skipped: addr=0x{full_address:08X} (outside target range)")
                    
                elif record_type == 0x01:  # End of File Record
                    print(f"    End of file record at line {line_num}")
                    break
                    
                elif record_type == 0x02:  # Extended Segment Address Record
                    extended_address = int(data_hex, 16) << 4
                    print(f"    Extended segment addr: 0x{extended_address:08X}")
                    
                elif record_type == 0x04:  # Extended Linear Address Record  
                    extended_address = int(data_hex, 16) << 16
                    print(f"    Extended linear addr: 0x{extended_address:08X}")
                    
                elif record_type == 0x05:  # Start Linear Address Record
                    entry_point = int(data_hex, 16)
                    print(f"    Entry point: 0x{entry_point:08X}")
                    
                else:
                    print(f"    Unknown record type {record_type:02X} at line {line_num}")
                    
            except (ValueError, IndexError) as e:
                print(f"[ERROR] Invalid HEX format at line {line_num}: {e}")
                continue
    
    if not memory_map:
        print("[ERROR] No valid data records found for target Flash address!")
        return bytes(), 0, 0, 0
    
    print(f"[*] Flash Memory range: 0x{min_addr:08X} - 0x{max_addr:08X}")
    print(f"[*] Binary data size: {len(memory_map)} bytes")
    if entry_point:
        print(f"[*] Entry point: 0x{entry_point:08X}")
    
    # Convert memory map to continuous binary data
    # Start from the minimum address (should be STM32_FLASH_BASE)
    bin_data = bytearray()
    
    # Create continuous binary from min_addr to max_addr
    for addr in range(min_addr, max_addr + 1):
        if addr in memory_map:
            bin_data.append(memory_map[addr])
        else:
            bin_data.append(0xFF)  # Fill gaps with 0xFF (erased flash state)
    
    # Show memory layout info
    print(f"[*] Vector Table (first 32 bytes):")
    preview_bytes = min(32, len(bin_data))
    for i in range(0, preview_bytes, 4):
        if i + 3 < len(bin_data):
            word_val = struct.unpack("<I", bin_data[i:i+4])[0]  # Little-endian
            if i == 0:
                print(f"    SP (0x{min_addr+i:08X}): 0x{word_val:08X} (Stack Pointer)")
            elif i == 4:
                print(f"    PC (0x{min_addr+i:08X}): 0x{word_val:08X} (Reset Vector)")
            else:
                print(f"    Vec[{i//4:2d}] (0x{min_addr+i:08X}): 0x{word_val:08X}")
    
    # Show first raw bytes
    preview_hex = ' '.join(f'{b:02X}' for b in bin_data[:preview_bytes])
    print(f"[*] Raw binary (first {preview_bytes} bytes): {preview_hex}")
    
    return bytes(bin_data), min_addr, max_addr, entry_point or 0

# =====================
# Helper function to find firmware file
# =====================
def find_firmware_file():
    """Find the first .hex or .bin file in the current directory."""
    script_dir = os.path.dirname(os.path.abspath(__file__))
    for ext in ['.hex', '.bin']:
        for f in os.listdir(script_dir):
            if f.endswith(ext):
                print(f"[*] Auto-detected firmware file: {f}")
                return os.path.join(script_dir, f)
    return None

# =====================
# Enhanced OTA sending function
# =====================
def send_stm32_ota(hex_file=None, chunk_size=64, enable_log=True, target_flash_addr=STM32_FLASH_BASE, 
                   include_flash_addr=True, session_id=None):
    """Send STM32 OTA update via serial with new header format"""
    
    # Generate random session ID if not provided
    if session_id is None:
        import random
        session_id = random.randint(0x1000, 0xFFFF)
    
    # Tự động tìm file firmware nếu không được chỉ định
    if hex_file is None:
        hex_file = find_firmware_file()
        if hex_file is None:
            print("[ERROR] No firmware file found in current directory!")
            print("        Please ensure you have a .hex or .bin file in the same folder as this script.")
            return
    
    # Setup log file
    log_file = None
    if enable_log:
        script_dir = os.path.dirname(os.path.abspath(__file__))
        log_file = os.path.join(script_dir, "stm32_ota_log.txt")
        init_log_file(log_file)
        print(f"[*] Logging enabled: {log_file}")
    
    print(f"[*] Reading firmware from: {hex_file}")
    
    try:
        # Convert HEX to STM32 binary
        fw_bin, start_addr, end_addr, entry_point = hex_to_stm32_binary(hex_file, target_flash_addr)
        
        if not fw_bin:
            print("[ERROR] No binary data extracted from HEX file!")
            return
            
    except FileNotFoundError:
        print(f"[ERROR] File not found: {hex_file}")
        return
    except Exception as e:
        print(f"[ERROR] Failed to read hex file: {e}")
        return
    
    fw_size = len(fw_bin)
    if fw_size > 65535:
        print(f"[ERROR] Firmware size ({fw_size} bytes) exceeds 65535 bytes limit!")
        print("        Consider using uint32_t fw_size or split into multiple segments")
        return
        
    fw_crc = crc_slow(fw_bin)  # Tính CRC của toàn bộ firmware binary
    total_chunks = (fw_size + chunk_size - 1) // chunk_size
    
    print(f"\n[*] STM32 OTA Session Information:")
    print(f"    Session ID:   0x{session_id:04X}")
    print(f"    Binary size:  {fw_size} bytes")
    print(f"    Flash start:  0x{start_addr:08X}")
    print(f"    Flash end:    0x{end_addr:08X}")
    print(f"    Entry point:  0x{entry_point:08X}")
    print(f"    Binary CRC:   0x{fw_crc:08X}")
    print(f"    Chunk size:   {chunk_size} bytes")
    print(f"    Total chunks: {total_chunks}")
    print(f"    Include flash addr: {'Yes' if include_flash_addr else 'No'}")
    
    try:
        with serial.Serial(SER_PORT, SER_BAUDRATE, timeout=TIMEOUT) as ser:
            print(f"\n[*] Serial opened on {SER_PORT} @ {SER_BAUDRATE}")
            time.sleep(0.1)  # Wait for serial to stabilize
            
            # 1. START - Bắt đầu quá trình OTA
            start_payload = b"\x00"  # Payload: 0x00 (1 byte)
            frame = build_frame(OTA_TYPE_START, start_payload)
            if enable_log:
                log_frame(log_file, "TX", "START", frame, "Begin STM32 OTA process")
            ser.write(frame)
            time.sleep(SLEEP_TIME)
            print("[TX] START")
            
            # 2. HEADER - Gửi thông tin firmware với session_id
            header_payload = pack_header(session_id, fw_size, chunk_size, fw_crc, 
                                       include_flash_addr, start_addr)
            frame = build_frame(OTA_TYPE_HEADER, header_payload)
            
            header_info = f"session_id=0x{session_id:04X}, fw_size={fw_size}, chunk_size={chunk_size}, total_chunks={total_chunks}, fw_crc=0x{fw_crc:08X}"
            if include_flash_addr:
                header_info += f", flash_addr=0x{start_addr:08X}"
                
            if enable_log:
                log_frame(log_file, "TX", "HEADER", frame, header_info)
                
            ser.write(frame)
            time.sleep(SLEEP_TIME)
            
            if include_flash_addr:
                print(f"[TX] HEADER (session=0x{session_id:04X}, fw_size={fw_size}, chunks={total_chunks}, flash=0x{start_addr:08X})")
            else:
                print(f"[TX] HEADER (session=0x{session_id:04X}, fw_size={fw_size}, chunks={total_chunks})")
            
            # 3. DATA - Gửi raw binary data theo chunks
            seq = 0
            
            print(f"\n[*] Sending {total_chunks} data chunks...")
            for i in range(0, fw_size, chunk_size):
                chunk = fw_bin[i:i+chunk_size]
                # Payload: SEQ (2 bytes) + [Flash Address (4 bytes, optional)] + Raw Binary Data
                if include_flash_addr:
                    chunk_addr = start_addr + i
                    payload = struct.pack(">H I", seq, chunk_addr) + chunk
                else:
                    payload = struct.pack(">H", seq) + chunk
                frame = build_frame(OTA_TYPE_DATA, payload)
                
                if enable_log:
                    chunk_addr = start_addr + i
                    extra_info = f"seq={seq}/{total_chunks-1}, size={len(chunk)} bytes"
                    if include_flash_addr:
                        extra_info += f", flash_addr=0x{chunk_addr:08X}"
                    log_frame(log_file, "TX", "DATA", frame, extra_info)
                
                ser.write(frame)
                time.sleep(SLEEP_TIME)
                
                # Progress indicator
                progress = (seq + 1) * 100 // total_chunks
                extra_info = f"[TX] DATA seq={seq:3d}/{total_chunks-1} [{progress:3d}%] size={len(chunk):2d} bytes"
                if include_flash_addr:
                    extra_info += f" @ 0x{chunk_addr:08X}"
                print(extra_info)
                seq += 1
            
            # 4. END - Kết thúc truyền data
            end_payload = b"\x01"  # Payload: 0x01 (1 byte)
            frame = build_frame(OTA_TYPE_END, end_payload)
            if enable_log:
                log_frame(log_file, "TX", "END", frame, "End of firmware binary data")
            ser.write(frame)
            time.sleep(SLEEP_TIME)
            print("[TX] END")
            
            # 5. SIGNAL_UPDATE - Yêu cầu STM32 thực hiện update
            update_info = struct.pack(">I I", start_addr, entry_point)  # Flash addr + Entry point
            frame = build_frame(OTA_TYPE_SIGNAL_UPDATE, update_info)
            if enable_log:
                log_frame(log_file, "TX", "SIGNAL_UPDATE", frame, 
                         f"flash_addr=0x{start_addr:08X}, entry_point=0x{entry_point:08X}")
            ser.write(frame)
            time.sleep(SLEEP_TIME)
            print(f"[TX] SIGNAL_UPDATE (flash_addr=0x{start_addr:08X}, entry=0x{entry_point:08X})")
            
    except serial.SerialException as e:
        print(f"[ERROR] Serial error: {e}")
        return
    except Exception as e:
        print(f"[ERROR] Unexpected error: {e}")
        return
    
    print(f"\n[*] STM32 OTA transmission completed successfully!")
    print(f"[*] Session 0x{session_id:04X}: {fw_size} bytes of raw binary data sent to STM32 Flash")
    if enable_log:
        print(f"[*] Check log file for detailed frame data: {log_file}")

# =====================
# Test CRC function
# =====================
def test_crc():
    """Test CRC calculation with known data"""
    test_data = b"123456789"
    crc_result = crc_slow(test_data)
    print(f"CRC of '{test_data.decode()}': 0x{crc_result:08X}")
    
    # Test với STM32 vector table
    stm32_vectors = bytes([0x00, 0x20, 0x00, 0x20, 0xED, 0x00, 0x00, 0x08])  # Typical STM32 start
    crc_vectors = crc_slow(stm32_vectors)
    print(f"CRC of STM32 vectors: 0x{crc_vectors:08X}")

# =====================
# Main
# =====================
if __name__ == "__main__":
    print("=== STM32 OTA Server với Raw Binary Data ===")
    
    # Test CRC trước
    print("\n[*] Testing CRC calculation:")
    test_crc()
    
    print(f"\n[*] STM32 Configuration:")
    print(f"    Flash Base: 0x{STM32_FLASH_BASE:08X}")
    print(f"    Flash Size: {STM32_FLASH_SIZE} bytes ({STM32_FLASH_SIZE//1024}KB)")
    print(f"    Page Size:  {STM32_PAGE_SIZE} bytes")
    
    print(f"\n[*] Serial Configuration:")
    print(f"    Port: {SER_PORT}")
    print(f"    Baudrate: {SER_BAUDRATE}")
    
    # Gửi STM32 OTA với tối ưu hóa và session ID
    print("\n=== Example Usage ===")
    print("# With flash address (16-byte header):")
    print('send_stm32_ota("firmware.hex", chunk_size=64, include_flash_addr=True, session_id=0x1234)')
    print("\n# Without flash address (12-byte header - bootloader uses default):")
    print('send_stm32_ota("firmware.hex", chunk_size=64, include_flash_addr=False)')
    print("\n# Auto-generate session ID:")
    print('send_stm32_ota("firmware.hex", chunk_size=64)')
    
    # Example call
    send_stm32_ota("firmware.hex", chunk_size=128, enable_log=True, include_flash_addr=False, session_id=0x1234)