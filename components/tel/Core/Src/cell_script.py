import re, time, serial
from influxdb_client import InfluxDBClient, Point
from influxdb_client.client.write_api import SYNCHRONOUS

SERIAL_PORT   = "/dev/ttyUSB0"
BAUDRATE      = 115200

INFLUX_URL    = "http://100.120.214.69"
INFLUX_ORG    = "UBC Solar"
INFLUX_BUCKET = "CAN_test"
INFLUX_TOKEN  = ""
GRAFANA_TOKEN = ""

def to_int_id(raw):
    if isinstance(raw, str) and raw.lower().startswith("0x"):
        return int(raw, 16)
    return int(raw)

def hex_to_bytes(s):
    return bytes.fromhex(s.replace(" ", "").replace("0x", ""))

def main():
    try:
        ser = serial.Serial(SERIAL_PORT, BAUDRATE, timeout=1)
    except Exception as e:
        # Your CAN class already emits a detailed exception; keep this quiet in hot path
        # print(f"[CAN-ERR] {e}")
        return False

def main_loop():
    client = InfluxDBClient(url=INFLUX_URL, org=INFLUX_ORG, token=INFLUX_TOKEN, timeout=10000)
    write_api = client.write_api(write_options=SYNCHRONOUS)

    with serial.Serial(SERIAL_PORT, BAUDRATE, timeout=1) as ser:
        print(f"INFLUX READY: {INFLUX_URL} org={INFLUX_ORG} bucket={INFLUX_BUCKET}")
        while True:
            raw = ser.readline()
            if not raw:
                continue
            # raw may already be the 21–22 bytes; sometimes the upstream prints hex text
            try:
                line = raw.decode('ascii', errors='ignore').strip()
            except Exception:
                line = raw.hex()

            if not handle_raw_hex_line(line, write_api):
                # If the upstream already gives you binary (not hex text),
                # you can fall back to raw.hex():
                handle_raw_hex_line(raw.hex(), write_api)

if __name__ == "__main__":
    main()