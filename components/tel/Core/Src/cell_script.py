#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import sys
import struct
import serial
import cantools
from datetime import datetime, timezone
from influxdb_client import InfluxDBClient, Point
# from influxdb_client.client.write_api import SYNCHRONOUS            # <<< CHANGED (no longer used)
from influxdb_client.client.write_api import WriteOptions             # <<< ADDED
import signal, time                                                   # <<< ADDED
import logging                                                        # <<< ADDED
# ---------------- CONFIG ----------------
DBC_FILE      = "/home/tonychen/brightside.dbc"
SERIAL_PORT   = "/dev/ttyUSB0"
BAUDRATE      = 115200*2
INFLUX_URL    = "http://100.120.214.69"
INFLUX_ORG    = "UBC Solar"
INFLUX_BUCKET = "CAN_test"
INFLUX_TOKEN  = ""
USE_NOW_TIME = True  # Use current time for Influx _time
FRAME_LEN = 21       # 8 (ts) + 1 (filler?) + 4 (id) + 8 (data)
# Batch settings
BATCH_SIZE = 1000                                                    # <<< ADDED
FLUSH_INTERVAL_S = 1.0                                               # <<< ADDED
# ---------------- SETUP ----------------
logging.basicConfig(level=logging.INFO)                              # <<< ADDED
try:
    ser = serial.Serial(SERIAL_PORT, BAUDRATE, timeout=1)
except Exception as e:
    raise RuntimeError(f"Failed to open {SERIAL_PORT}: {e}")
db = cantools.database.load_file(DBC_FILE)
# Enable gzip + async batch writer
client = InfluxDBClient(                                             # <<< CHANGED
    url=INFLUX_URL, org=INFLUX_ORG, token=INFLUX_TOKEN,
    enable_gzip=True                                                 # <<< ADDED
)
write_api = client.write_api(                                        # <<< CHANGED
    write_options=WriteOptions(
        batch_size=BATCH_SIZE,
        flush_interval=int(FLUSH_INTERVAL_S * 1000),  # ms
        jitter_interval=100,                           # ms
        retry_interval=5000,                           # ms
        max_retries=5,
        max_retry_delay=30_000,
        exponential_base=2
    )
)
print(f"INFLUX READY: {INFLUX_URL} org={INFLUX_ORG} bucket={INFLUX_BUCKET}")
print(f"Listening for CAN messages on {SERIAL_PORT} @ {BAUDRATE}... (USE_NOW_TIME={USE_NOW_TIME})")
# ---- Health check + smoke write ----
print("Pinging Influx...")
try:
    ok = client.ping()
    if ok:
        print("Influx ping OK")
    else:
        print("Influx ping failed")
except Exception as e:
    print(f"Influx ping threw exception: {e}")
print("Writing smoke_test point...")
try:
    smoke = (Point("smoke_test")
             .tag("host", "raspi")
             .field("value", 1.0)
             .time(datetime.now(timezone.utc)))
    write_api.write(bucket=INFLUX_BUCKET, org=INFLUX_ORG, record=smoke)
    write_api.flush()                                                # <<< ADDED (force flush so you can see it immediately)
    print("Wrote smoke_test point. Check Data Explorer → measurement=smoke_test (Last 15m).")
except Exception as e:
    print(f"Smoke write failed: {e}")
# ---------------- TIMESTAMP PARSER ----------------
def parse_timestamp_seconds(ts8: bytes) -> float:
    try:
        return float(struct.unpack(">d", ts8)[0])
    except Exception:
        pass
    try:
        return float(struct.unpack(">Q", ts8)[0])
    except Exception:
        pass
    try:
        ms = struct.unpack(">Q", ts8)[0]
        return float(ms) / 1000.0
    except Exception:
        pass
    return struct.unpack(">d", ts8)[0]
# ---------------- CORE BUILD ----------------
def build_output_dict(source, message_obj, measurements, hex_id, ts_seconds, raw_bytes):
    data = {
        "Source": [],
        "Class": [],
        "Measurement": [],
        "Value": [],
        "Timestamp": [],
        "display_data": {
            "ROW": {"Raw Hex": [raw_bytes.hex()]},
            "COL": {"Hex_ID": [], "Source": [], "Class": [], "Measurement": [], "Value": [], "Timestamp": []}
        }
    }
    for name, val in measurements.items():
        data["Source"].append(source)
        data["Class"].append(message_obj.name)
        data["Measurement"].append(name)
        data["Value"].append(val)
        data["Timestamp"].append(ts_seconds)
        data["display_data"]["COL"]["Hex_ID"].append(hex_id)
        data["display_data"]["COL"]["Source"].append(source)
        data["display_data"]["COL"]["Class"].append(message_obj.name)
        data["display_data"]["COL"]["Measurement"].append(name)
        data["display_data"]["COL"]["Value"].append(val)
        data["display_data"]["COL"]["Timestamp"].append(
            datetime.fromtimestamp(ts_seconds).strftime('%Y-%m-%d %H:%M:%S.%f')[:-3]
        )
    return data
def try_decode_layout(raw21: bytes, layout: str):
    if layout == "with_filler":
        ts_bytes, id_bytes, data_bytes = raw21[0:8], raw21[9:13], raw21[13:21]
    elif layout == "no_filler":
        ts_bytes, id_bytes, data_bytes = raw21[0:8], raw21[8:12], raw21[12:20]
    else:
        raise ValueError("Unknown layout")
    ts_seconds = parse_timestamp_seconds(ts_bytes)
    can_id = int.from_bytes(id_bytes, "big")
    hex_id = "0x" + hex(can_id)[2:].upper()
    message_obj = db.get_message_by_frame_id(can_id)
    measurements = db.decode_message(can_id, bytearray(data_bytes))
    sources = getattr(message_obj, "senders", []) or []
    source = sources[0] if sources else "UNKNOWN"
    return build_output_dict(source, message_obj, measurements, hex_id, ts_seconds, raw21)
def decode_frame(raw21: bytes):
    try:
        return try_decode_layout(raw21, "with_filler")
    except Exception:
        return try_decode_layout(raw21, "no_filler")
# ---------------- BATCH BUFFER + HELPERS ----------------
_points_buf = []                                                     # <<< ADDED
_last_flush = time.time()                                            # <<< ADDED
def _make_points(parsed: dict):                                      # <<< ADDED
    """Build one Point per measurement from parsed dict."""
    points = []
    can_ts = parsed["Timestamp"][0]
    ts_influx = datetime.now(timezone.utc) if USE_NOW_TIME else datetime.fromtimestamp(can_ts, tz=timezone.utc)
    src = parsed["Source"][0]
    cls = parsed["Class"][0]
    for name, val in zip(parsed["Measurement"], parsed["Value"]):
        if isinstance(val, bool):
            val = 1.0 if val else 0.0
        elif not isinstance(val, (int, float)):
            continue
        p = (Point(src)
             .tag("class", cls)                   # keep tag count moderate
             .field(name, float(val))             # each signal as field
             .field("can_timestamp", float(can_ts))
             .time(ts_influx))
        points.append(p)
    return points
def _flush_if_needed(force: bool = False):                           # <<< ADDED
    """Flush when batch is full or interval elapsed."""
    global _points_buf, _last_flush
    if force or len(_points_buf) >= BATCH_SIZE or (time.time() - _last_flush) >= FLUSH_INTERVAL_S:
        if _points_buf:
            try:
                write_api.write(bucket=INFLUX_BUCKET, org=INFLUX_ORG, record=_points_buf)
            except Exception as e:
                # Keep it loud enough to notice, but not per-point noisy
                logging.warning(f"Batch write failed (size={len(_points_buf)}): {e}")
            _points_buf = []
            _last_flush = time.time()
def _shutdown(*_):                                                   # <<< ADDED
    """Ensure buffers are flushed on exit."""
    try:
        _flush_if_needed(force=True)
        write_api.flush()
    finally:
        try:
            write_api.close()
        except Exception:
            pass
        try:
            client.close()
        except Exception:
            pass
        sys.exit(0)
signal.signal(signal.SIGINT, _shutdown)                              # <<< ADDED
signal.signal(signal.SIGTERM, _shutdown)                             # <<< ADDED
# ---------------- INFLUX WRITE (BATCHED) ----------------
def write_to_influx(parsed: dict):                                   # <<< CHANGED (now buffers instead of per-point writes)
    global _points_buf
    pts = _make_points(parsed)                                       # build points for this CAN frame
    if pts:
        _points_buf.extend(pts)
    _flush_if_needed()
"""
Purpose: Processes the message by splitting it into parts and returning the parts and the buffer
Parameters:
    message - The total chunk read from the serial stream
    buffer - the buffer to be added to the start of the message
Returns (tuple):
    parts - the fully complete messages of the total chunk read
    buffer - leftover chunk that is not a message
"""
def process_message(message: str, buffer: str = "") -> list:
    # Remove 00 0a from the start if present
    if message.startswith("000a"):
        message = message[4:]
    elif message.startswith("0a"):
        message = message[2:]
    # Add buffer to the start of the message
    message = buffer + message
    # Split the message by 0d 0a. TEL board sends messages ending with \r\n which is 0d0a in hex. Use as delimeter
    parts = message.split("0d0a")
    if len(parts[-1]) != 30 or len(parts[-1]) != 396 or len(parts[-1]) != 44:
        buffer = parts.pop()
    try:
        parts = [part + "0d0a" for part in parts if len(part) == 30 or len(part) == 396 or len(part) == 44]
    except ValueError as e:
        print(f"{ANSI_RED}Failed to split message: {str([part for part in parts])}{ANSI_ESCAPE}"
              f"    ERROR: {e}")
        return [], buffer
    return  [bytes.fromhex(part) for part in parts] , buffer
# ---------------- RESYNCING SERIAL LOOP ----------------
CHUNK_SIZE = 24 * 21        # 21 CAN messages from serial at a time.
# Optional: simple 1 Hz heartbeat for throughput visibility         # <<< ADDED
_last_log = time.time()                                              # <<< ADDED
_ingest = 0                                                          # <<< ADDED
def run():
    global _last_log, _ingest                                       # <<< ADDED
    buffer = ""
    buf = bytearray()
    while True:
        chunk = ser.read(CHUNK_SIZE)
        if not chunk:
            _flush_if_needed()                                      # <<< ADDED (time-based flush even if idle)
            # heartbeat
            now = time.time()                                       # <<< ADDED
            if now - _last_log >= 1.0:                              # <<< ADDED
                logging.info("ingest=%d buf=%d", _ingest, len(_points_buf))
                _ingest = 0
                _last_log = now
            continue
        chunk = chunk.hex()
        parts, buffer = process_message(chunk, buffer)
        for part in parts:
            try:
                parsed = decode_frame(part)
                _ingest += 1                                        # <<< ADDED
                write_to_influx(parsed)                             # (batched)
            except Exception as e:
                print("ERROR", e)
if __name__ == "__main__":
    try:
        run()
    except KeyboardInterrupt:
        print("EXIT")
        _shutdown()                                                 # <<< CHANGED (ensure flush/close)