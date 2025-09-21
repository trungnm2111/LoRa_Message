#!/usr/bin/env python3
"""
Robust LoRa serial sender + reader
- Reset input buffer before send
- ser.flush() after write
- Poll for incoming bytes up to RESPONSE_TIMEOUT
- Use read_all() when available
- Try toggling DTR/RTS if requested
- Log everything to file and print to console
"""
import os
import time
import struct
import zlib
import binascii
from datetime import datetime

import serial

# -----------------------
# Config
# -----------------------
SER_PORT = "COM15"           # change if needed
SER_BAUDRATE = 115200        # change if needed
SER_TIMEOUT = 0.05           # per-read timeout (seconds) for pyserial
RESPONSE_TIMEOUT = 5.0       # how long to wait total for response after each send
INTER_FRAME_DELAY = 0.05     # small delay after send
LOG_PATH = "lora_tx_log.txt"
HEX_FILE = "firmware.hex"
DATA_CHUNK_SIZE = 128

# -----------------------
# Helpers
# -----------------------
def now_ts():
    return datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f")[:-3]

def hex_str(b: bytes) -> str:
    return ' '.join(f'{x:02X}' for x in b)

def append_log(line: str):
    with open(LOG_PATH, "a", encoding="utf-8") as f:
        f.write(line + "\n")

# -----------------------
# Intel HEX loader (simple)
# -----------------------
def parse_hex_line(line):
    if not line.startswith(":"):
        return None
    byte_count = int(line[1:3], 16)
    address = int(line[3:7], 16)
    record_type = int(line[7:9], 16)
    data = line[9:9 + byte_count * 2]
    return record_type, address, binascii.unhexlify(data)

def load_firmware_from_hex(path):
    if not os.path.exists(path):
        raise FileNotFoundError(path)
    fw = bytearray()
    with open(path, "r") as f:
        for line in f:
            line = line.strip()
            if not line: 
                continue
            rec = parse_hex_line(line)
            if not rec:
                continue
            rec_type, addr, data = rec
            if rec_type == 0x00:
                fw.extend(data)
    return bytes(fw)

# -----------------------
# Frame builder (your format)
# -----------------------
def create_lora_frame(type_packet, type_message, payload: bytes) -> bytes:
    START_BYTE = 0xAA
    END_BYTE = 0xBB
    length = len(payload)
    length_L = length & 0xFF
    length_H = (length >> 8) & 0xFF
    frame = bytearray()
    frame.append(START_BYTE)
    frame.append(type_packet)
    frame.append(type_message)
    frame.append(length_L)
    frame.append(length_H)
    frame.extend(payload)
    crc = zlib.crc32(payload) & 0xFFFFFFFF
    frame.extend(struct.pack('>I', crc))
    frame.append(END_BYTE)
    return bytes(frame)

# -----------------------
# Robust send & read function
# -----------------------
def send_frame_and_read(ser: serial.Serial, label: str, frame: bytes,
                        response_timeout: float = RESPONSE_TIMEOUT,
                        inter_frame_delay: float = INTER_FRAME_DELAY,
                        toggle_dtr_rts: bool = False):
    """
    Send `frame` then poll for response for up to response_timeout seconds.
    Returns bytes (may be empty b'').
    """
    try:
        # Optional: toggle DTR/RTS to try to avoid reset behaviour
        if toggle_dtr_rts:
            try:
                ser.dtr = False
                ser.rts = False
                time.sleep(0.05)
            except Exception:
                pass

        # Clear old data
        try:
            ser.reset_input_buffer()
        except Exception:
            pass

        # Write
        written = ser.write(frame)
        ser.flush()
        ts = now_ts()
        msg = f"[{ts}] TX {label} bytes={written} -> {hex_str(frame)}"
        print(msg)
        append_log(msg)

        # Small wait then poll
        start = time.time()
        collected = bytearray()
        last_data_time = None

        while True:
            # If timeout exceeded and no data arrived (or no new data for 0.3s after some arrived), break
            now = time.time()
            if (now - start) > response_timeout:
                break

            # read what is available
            try:
                n = ser.in_waiting
            except Exception:
                n = 0

            if n:
                # Prefer read_all if available (pyserial >= 3.0)
                try:
                    data = ser.read(n)
                except Exception:
                    try:
                        data = ser.read_all()
                    except Exception:
                        data = ser.read(n)  # fallback
                if data:
                    collected.extend(data)
                    last_data_time = time.time()
                    # reset the timeout window if you want to wait after last byte:
                    # start = time.time()
            else:
                # no data available now; if we've previously received data and there's been a small silence, consider done
                if last_data_time and (time.time() - last_data_time) > 0.25:
                    break

            time.sleep(0.01)

        # small inter-frame gap
        time.sleep(inter_frame_delay)

        # Log response
        if collected:
            ts = now_ts()
            resp_msg = f"[{ts}] RX {label}: {hex_str(bytes(collected))} ({len(collected)} bytes)"
            print(resp_msg)
            append_log(resp_msg)
            return bytes(collected)
        else:
            ts = now_ts()
            no_msg = f"[{ts}] RX {label}: No response (in_waiting final={getattr(ser, 'in_waiting', 'N/A')})"
            print(no_msg)
            append_log(no_msg)
            return b""
    except Exception as e:
        err = f"[{now_ts()}] ERROR send_frame_and_read {label}: {e}"
        print(err)
        append_log(err)
        return b""

# -----------------------
# Diagnostics helper
# -----------------------
def serial_diagnostics(ser: serial.Serial):
    try:
        s = f"Serial port: {ser.port}, baud={ser.baudrate}, bytesize={ser.bytesize}, parity={ser.parity}, stopbits={ser.stopbits}, timeout={ser.timeout}"
        print(s); append_log(s)
        try:
            ser.reset_input_buffer(); ser.reset_output_buffer()
            append_log(f"[{now_ts()}] Buffers reset for diagnostics.")
        except Exception as e:
            append_log(f"[{now_ts()}] Buffer reset failed: {e}")
        # Poll in_waiting for a short time to show if device is spitting data spontaneously
        for i in range(20):
            try:
                n = ser.in_waiting
            except Exception:
                n = 0
            if n:
                d = ser.read(n)
                append_log(f"[{now_ts()}] Diagnostic read {len(d)} bytes: {hex_str(d)}")
                print(f"[Diag] read {len(d)} bytes: {hex_str(d)}")
            time.sleep(0.05)
    except Exception as e:
        append_log(f"[{now_ts()}] Diagnostics error: {e}")

# -----------------------
# Main sequence
# -----------------------
def main():
    # init log
    with open(LOG_PATH, "w", encoding="utf-8") as f:
        f.write(f"=== LoRa TX Log - {now_ts()} ===\n")

    # load firmware
    try:
        fw = load_firmware_from_hex(HEX_FILE)
        append_log(f"[{now_ts()}] Loaded firmware {HEX_FILE}: {len(fw)} bytes")
        print(f"Loaded firmware {len(fw)} bytes")
    except Exception as e:
        err = f"[{now_ts()}] Failed to load firmware: {e}"
        print(err)
        append_log(err)
        return

    # build frames
    frame_start = create_lora_frame(0x02, 0x00, bytes([0x00]))
    session_id = struct.pack('>H', 0x1234)
    fw_size = struct.pack('>I', len(fw))
    chunk_size = struct.pack('>H', DATA_CHUNK_SIZE)
    total_chunks = struct.pack('>H', (len(fw) + DATA_CHUNK_SIZE - 1) // DATA_CHUNK_SIZE)
    fw_crc = struct.pack('>I', zlib.crc32(fw) & 0xFFFFFFFF)
    header_payload = session_id + fw_size + chunk_size + total_chunks + fw_crc + bytes([0x27, 0x2C, 0xA0])
    frame_header = create_lora_frame(0x02, 0x01, header_payload)
    frames_data = []
    for i in range(0, len(fw), DATA_CHUNK_SIZE):
        chunk = fw[i:i+DATA_CHUNK_SIZE]
        seq = struct.pack('>H', i // DATA_CHUNK_SIZE)
        payload = seq + chunk
        frames_data.append(create_lora_frame(0x02, 0x02, payload))
    frame_end = create_lora_frame(0x02, 0x04, bytes([0x01, 0x01]))

    # open serial
    try:
        ser = serial.Serial(port=SER_PORT,
                            baudrate=SER_BAUDRATE,
                            timeout=SER_TIMEOUT,
                            bytesize=serial.EIGHTBITS,
                            parity=serial.PARITY_NONE,
                            stopbits=serial.STOPBITS_ONE)
        print(f"Opened serial {SER_PORT}@{SER_BAUDRATE}")
        append_log(f"[{now_ts()}] Opened {SER_PORT}@{SER_BAUDRATE}")
    except Exception as e:
        err = f"[{now_ts()}] Cannot open serial port {SER_PORT}: {e}"
        print(err)
        append_log(err)
        return

    try:
        # Diagnostic quick peek (optional, helpful to see spontaneous prints)
        serial_diagnostics(ser)

        # START
        send_frame_and_read(ser, "TX START", frame_start, response_timeout=RESPONSE_TIMEOUT)

        # HEADER
        send_frame_and_read(ser, "TX HEADER", frame_header, response_timeout=RESPONSE_TIMEOUT)

        # DATA frames (example: send first 10 only for quick test; remove slice to send all)
        total = len(frames_data)
        for idx, frame in enumerate(frames_data):
            lbl = f"TX DATA {idx+1}/{total}"
            resp = send_frame_and_read(ser, lbl, frame, response_timeout=RESPONSE_TIMEOUT)
            # if you want to stop early for debug, uncomment:
            # if idx >= 9: break

        # END
        send_frame_and_read(ser, "TX END", frame_end, response_timeout=RESPONSE_TIMEOUT)

    except Exception as e:
        append_log(f"[{now_ts()}] Runtime error: {e}")
        print(f"Runtime error: {e}")
    finally:
        try:
            ser.close()
            append_log(f"[{now_ts()}] Serial closed.")
            print("Serial closed.")
        except Exception:
            pass

# -----------------------
# Run
# -----------------------
if __name__ == "__main__":
    main()
