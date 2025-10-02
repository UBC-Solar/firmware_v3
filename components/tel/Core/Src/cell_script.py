#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import struct
import serial
import cantools
from datetime import datetime, timezone
from influxdb_client import InfluxDBClient, Point
from influxdb_client.client.write_api import SYNCHRONOUS

# ---------------- CONFIG ----------------
DBC_FILE      = "/home/tonychen/brightside.dbc"
SERIAL_PORT   = "/dev/ttyUSB0"
BAUDRATE      = 115200
INFLUX_URL    = "http://100.120.214.69"
INFLUX_ORG    = "UBC Solar"
INFLUX_BUCKET = "CAN_test"
INFLUX_TOKEN  = ""

USE_NOW_TIME = True  # Use current time for Influx _time
FRAME_LEN = 21       # 8 (ts) + 1 (filler?) + 4 (id) + 8 (data)

# ---------------- SETUP ----------------
try:
    ser = serial.Serial(SERIAL_PORT, BAUDRATE, timeout=1)
except Exception as e:
    raise RuntimeError(f"Failed to open {SERIAL_PORT}: {e}")

db = cantools.database.load_file(DBC_FILE)

client = InfluxDBClient(url=INFLUX_URL, org=INFLUX_ORG, token=INFLUX_TOKEN)
write_api = client.write_api(write_options=SYNCHRONOUS)

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

# ---------------- INFLUX WRITE ----------------
def write_to_influx(parsed: dict):
    can_ts = parsed["Timestamp"][0]
    ts_influx = datetime.now(timezone.utc) if USE_NOW_TIME else datetime.fromtimestamp(can_ts, tz=timezone.utc)

    for name, val in zip(parsed["Measurement"], parsed["Value"]):
        if isinstance(val, bool):
            val = 1.0 if val else 0.0
        elif not isinstance(val, (int, float)):
            continue

        point = (Point(parsed["Source"][0])
                 .tag("class",  parsed["Class"][0])
                 .field(name, float(val))
                 .field("can_timestamp", float(can_ts))
                 .time(ts_influx))
        try:
            write_api.write(bucket=INFLUX_BUCKET, org=INFLUX_ORG, record=point)
        except Exception as e:
            print(f"Write failed: {e}")


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
    return [bytes.fromhex(part).decode('latin-1') for part in parts] , buffer


# ---------------- RESYNCING SERIAL LOOP ----------------
CHUNK_SIZE = 24 * 21        # 21 CAN messages from serial at a time.
buffer = ""
def run():
    buf = bytearray()
    while True:
        chunk = ser.read(CHUNK_SIZE)
        if not chunk:
            continue
        # buf.extend(chunk)
        chunk = chunk.hex()
        parts, buffer = process_message(chunk, buffer)

        for part in parts:
            try:
                parsed = decode_frame(part)

                print(f"OK id={parsed['display_data']['COL']['Hex_ID'][0]} "
                      f"src={parsed['Source'][0]} cls={parsed['Class'][0]} "
                      f"val={parsed['Value'][0]} ts={parsed['Timestamp'][0]:.3f}")

                write_to_influx(parsed)

            except Exception as e:
                print("ERROR", e)

if __name__ == "__main__":
    try:
        run()
    except KeyboardInterrupt:
        print("EXIT")
        try:
            client.close()
        except Exception:
            pass
        sys.exit(0)
