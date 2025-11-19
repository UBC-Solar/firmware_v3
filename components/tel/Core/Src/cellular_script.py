#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import sys
import struct
import serial
import cantools
from datetime import datetime, timezone
from influxdb_client import InfluxDBClient, Point
from influxdb_client.client.write_api import WriteOptions             # ADDED
import signal, time                                                   # ADDED
import logging                                                        # ADDED
# ---------------- CONFIG ----------------
DBC_FILE      = "/home/tonychen/brightside.dbc"
SERIAL_PORT   = "/dev/ttyUSB0"
BAUDRATE      = 115200*2
INFLUX_URL    = "http://100.120.214.69"
INFLUX_ORG    = "UBC Solar"
INFLUX_BUCKET = "CAN_test"
INFLUX_TOKEN  = "token"
USE_NOW_TIME = True  # Use current time for Influx _time
FRAME_LEN = 21       # 8 (ts) + 1 (filler?) + 4 (id) + 8 (data)
# Batch settings
INF_BATCH_SIZE = 1000                                                    # ADDED
INF_FLUSH_INTERVAL_S = 1.0                                               # ADDED

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s"
)
# ========================= SETUP =========================
try:
    ser = serial.Serial(SERIAL_PORT, BAUDRATE, timeout=1, rtscts=False)
except Exception as e:
    raise RuntimeError(f"Failed to open {SERIAL_PORT}: {e}")
db = cantools.database.load_file(DBC_FILE)
client = InfluxDBClient(
    url=INFLUX_URL,
    org=INFLUX_ORG,
    token=INFLUX_TOKEN,
    enable_gzip=False,  # save CPU on Pi / low-power hardware
)
write_api = client.write_api(
    write_options=WriteOptions(
        batch_size=INF_BATCH_SIZE,
        flush_interval=int(INF_FLUSH_INTERVAL_S * 1000),  # ms
        jitter_interval=100,
        retry_interval=5000,
        max_retries=5,
        max_retry_delay=30_000,
        exponential_base=2,
    )
)
print(f"INFLUX READY: {INFLUX_URL} org={INFLUX_ORG} bucket={INFLUX_BUCKET}")
print(f"Listening on {SERIAL_PORT} @ {BAUDRATE} (USE_NOW_TIME={USE_NOW_TIME})")
# Health check + smoke write
try:
    ok = client.ping()
    print("Influx ping OK" if ok else "Influx ping failed")
    smoke = (Point("smoke_test").tag("host", "raspi")
             .field("value", 1.0)
             .time(datetime.now(timezone.utc)))
    write_api.write(bucket=INFLUX_BUCKET, org=INFLUX_ORG, record=smoke)
    write_api.flush()
    print("Wrote smoke_test point.")
except Exception as e:
    print(f"Influx test failed: {e}")
# ========================= TIMESTAMP PARSER =========================
def parse_timestamp_seconds(ts8: bytes) -> float:
    """Try several formats for the 8-byte timestamp."""
    try:
        return float(struct.unpack(">d", ts8)[0])   # big-endian double
    except Exception:
        pass
    try:
        return float(struct.unpack(">Q", ts8)[0])   # uint64 seconds
    except Exception:
        pass
    try:
        ms = struct.unpack(">Q", ts8)[0]           # uint64 ms
        return float(ms) / 1000.0
    except Exception:
        pass
    # fallback
    return struct.unpack(">d", ts8)[0]
# ========================= DBC / DECODE =========================
_MSG_CACHE = {}
def _get_db_message(can_id: int):
    """Cache DBC message objects by CAN ID for speed."""
    msg = _MSG_CACHE.get(can_id)
    if msg is None:
        msg = db.get_message_by_frame_id(can_id)
        _MSG_CACHE[can_id] = msg
    return msg
def try_decode_layout(raw21: bytes, layout: str):
    """
    Decode a 21-byte frame using one of two known layouts:
    - with_filler: [0:8]=ts, [9:13]=id, [13:21]=payload
    - no_filler:   [0:8]=ts, [8:12]=id, [12:20]=payload
    """
    if layout == "with_filler":
        ts_bytes, id_bytes, data_bytes = raw21[0:8], raw21[9:13], raw21[13:21]
    elif layout == "no_filler":
        ts_bytes, id_bytes, data_bytes = raw21[0:8], raw21[8:12], raw21[12:20]
    else:
        raise ValueError("Unknown layout")
    ts_seconds = parse_timestamp_seconds(ts_bytes)
    can_id = int.from_bytes(id_bytes, "big")
    msg = _get_db_message(can_id)
    measurements = msg.decode(bytearray(data_bytes))
    sources = getattr(msg, "senders", []) or []
    source = sources[0] if sources else "UNKNOWN"
    cls_name = msg.name
    return source, cls_name, ts_seconds, measurements
def decode_frame(raw21: bytes):
    """Try both layouts for a 21-byte frame."""
    try:
        return try_decode_layout(raw21, "with_filler")
    except Exception:
        return try_decode_layout(raw21, "no_filler")
# ========================= INFLUX POINT BUILDER =========================
def make_point(source: str, cls_name: str, ts_seconds: float, measurements: dict) -> Point:
    """Build a single multi-field Point for one CAN frame."""
    if USE_NOW_TIME:
        ts_influx = datetime.now(timezone.utc)
    else:
        ts_influx = datetime.fromtimestamp(ts_seconds, tz=timezone.utc)
    p = Point(source).tag("class", cls_name)
    for name, val in measurements.items():
        if isinstance(val, bool):
            val = 1.0 if val else 0.0
        elif not isinstance(val, (int, float)):
            # skip non-numeric fields
            continue
        p.field(name, float(val))
    # always record the original CAN timestamp too
    p.field("can_timestamp", float(ts_seconds))
    p.time(ts_influx)
    return p
def write_to_influx(source: str, cls_name: str, ts_seconds: float,
                    measurements: dict, counters: dict):
    """Write one frame as one Point to Influx (async)."""
    try:
        point = make_point(source, cls_name, ts_seconds, measurements)
        write_api.write(bucket=INFLUX_BUCKET, org=INFLUX_ORG, record=point)
        counters["written"] += 1
    except Exception as e:
        counters["write_errors"] += 1
        if counters["write_errors"] <= 10 or counters["write_errors"] % 1000 == 0:
            logging.warning("Influx write error (count=%d): %r",
                            counters["write_errors"], e)
# ========================= ORIGINAL HEX SPLITTER =========================
_HEX_OK = set("0123456789abcdefABCDEF")
def process_message(message_hex: str, buffer_hex: str = ""):
    """
    Your original robust ASCII-hex splitter, kept essentially the same.
    - message_hex: hex string from chunk.hex()
    - buffer_hex:  hex tail from previous chunk that didn't end with 0d0a
    Returns:
      (list of 21-byte frames as bytes, new_buffer_hex)
    """
    s = buffer_hex + message_hex
    parts = s.split("0d0a")  # split on CRLF in hex form
    buffer_hex = parts.pop() if parts else ""
    frames = []
    for part in parts:
        if not part:
            continue
        part = part.replace(" ", "")
        n = len(part)
        if n < 42:
            continue
        j = 0
        while j <= n - 42:
            seg = part[j:j+42]
            if all(c in _HEX_OK for c in seg):
                try:
                    frames.append(bytes.fromhex(seg))
                    j += 42
                    continue
                except ValueError:
                    pass
            j += 1
    return frames, buffer_hex
# ========================= STATS + SHUTDOWN =========================
counters = {
    "frames_seen":   0,
    "decoded":       0,
    "decode_errors": 0,
    "written":       0,
    "write_errors":  0,
}
_start_time = time.time()
_last_log   = _start_time
def _shutdown(*_):
    try:
        logging.info("Shutting down, flushing Influx...")
        write_api.flush()
        elapsed = max(time.time() - _start_time, 1e-6)
        logging.info(
            "FINAL STATS: frames_seen=%d decoded=%d written=%d "
            "decode_errors=%d write_errors=%d uptime=%.1fs avg_decoded/s=%.1f",
            counters["frames_seen"], counters["decoded"], counters["written"],
            counters["decode_errors"], counters["write_errors"],
            elapsed, counters["decoded"] / elapsed
        )
    finally:
        try:
            write_api.close()
        except Exception:
            pass
        try:
            client.close()
        except Exception:
            pass
        try:
            ser.close()
        except Exception:
            pass
        sys.exit(0)
signal.signal(signal.SIGINT, _shutdown)
signal.signal(signal.SIGTERM, _shutdown)
# ========================= MAIN LOOP =========================
def run():
    global _last_log
    logging.info("CAN ingest loop started.")
    buffer_hex = ""
    while True:
        chunk = ser.read(CHUNK_SIZE)
        if not chunk:
            # idle: just log stats once per second
            now = time.time()
            if now - _last_log >= 1.0:
                elapsed = max(now - _start_time, 1e-6)
                ingest_rate = counters["decoded"] / elapsed
                logging.info(
                    "stats: frames_seen=%d decoded=%d written=%d "
                    "decode_err=%d write_err=%d avg_decoded/s=%.1f",
                    counters["frames_seen"], counters["decoded"], counters["written"],
                    counters["decode_errors"], counters["write_errors"],
                    ingest_rate
                )
                _last_log = now
            continue
        # 1) convert raw chunk to hex string
        hex_chunk = chunk.hex()
        # 2) use your proven splitter to extract 21-byte frames
        frames, buffer_hex = process_message(hex_chunk, buffer_hex)
        # 3) decode & send each frame
        for raw21 in frames:
            counters["frames_seen"] += 1
            try:
                source, cls_name, ts_seconds, measurements = decode_frame(raw21)
                counters["decoded"] += 1
            except Exception as e:
                counters["decode_errors"] += 1
                if counters["decode_errors"] <= 10 or counters["decode_errors"] % 1000 == 0:
                    logging.warning(
                        "Decode error (count=%d): %r", counters["decode_errors"], e
                    )
                continue
            write_to_influx(source, cls_name, ts_seconds, measurements, counters)
        # 4) light stats once per second
        now = time.time()
        if now - _last_log >= 1.0:
            elapsed = max(now - _start_time, 1e-6)
            ingest_rate = counters["decoded"] / elapsed
            logging.info(
                "stats: frames_seen=%d decoded=%d written=%d "
                "decode_err=%d write_err=%d avg_decoded/s=%.1f",
                counters["frames_seen"], counters["decoded"], counters["written"],
                counters["decode_errors"], counters["write_errors"],
                ingest_rate
            )
            _last_log = now
if __name__ == "__main__":
    try:
        run()
    except KeyboardInterrupt:
        _shutdown()