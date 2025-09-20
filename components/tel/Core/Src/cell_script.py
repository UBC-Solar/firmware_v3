import time, json, serial, cantools
from influxdb_client import InfluxDBClient, Point
from influxdb_client.client.write_api import SYNCHRONOUS

SERIAL_PORT   = "/dev/ttyUSB0"
BAUDRATE      = 115200

DBC_FILE      = "/home/tonychen/brightside.dbc"

INFLUX_URL    = "influxdb.telemetry.ubcsolar.com"
GRAFANA_URL   = "http://localhost:3000"
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
        raise RuntimeError(f"Failed to open {SERIAL_PORT}: {e}")

    db = cantools.database.load_file(DBC_FILE)
    client = InfluxDBClient(url=INFLUX_URL, org=INFLUX_ORG, token=INFLUX_TOKEN)
    write_api = client.write_api(write_options=SYNCHRONOUS)

    print(f"INFLUX READY: {INFLUX_URL} org={INFLUX_ORG} bucket={INFLUX_BUCKET}")

    try:
        while True:
            line = ser.readline().decode(errors="ignore").strip()
            if not line:
                continue
            try:
                obj = json.loads(line)
            except json.JSONDecodeError:
                continue

            if "id" not in obj or "data" not in obj:
                continue

            can_id = to_int_id(obj["id"])
            try:
                data = hex_to_bytes(obj["data"])
            except ValueError:
                continue

            dlc = int(obj.get("dlc", len(data)))
            if dlc != len(data):
                continue

            t = float(obj.get("t", time.time()))
            ts_ns = int(t * 1e9)

            try:
                msg = db.get_message_by_frame_id(can_id)
            except KeyError:
                continue

            try:
                decoded = msg.decode(data)
                print(f"[DECODE] {msg.name} 0x{can_id:X} -> {decoded}")
            except Exception:
                continue

            points = []
            for sig_name, val in decoded.items():
                if isinstance(val, (int, float)):
                    points.append(
                        Point(msg.name)
                        .tag("class", msg.name)
                        .field(sig_name, float(val))
                        .time(ts_ns)
                    )

            if points:
                try:
                    write_api.write(bucket=INFLUX_BUCKET, org=INFLUX_ORG, record=points)
                except Exception:
                    pass

    except KeyboardInterrupt:
        print("\nEnd Script")
    finally:
        try:
            write_api.close(); client.close(); ser.close()
        except Exception:
            pass

if __name__ == "__main__":
    main()
