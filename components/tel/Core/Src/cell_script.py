import time
import json
import serial
import cantools
from influxdb_client import InfluxDBClient, Point
from influxdb_client.client.write_api import SYNCHRONOUS

SERIAL_PORT     = "/dev/ttyUSB0"
BAUDRATE        = 115200

DBC_FILE        = ""

INFLUX_URL      = "https://localhost:8086"
INFLUX_ORG      = "UBC Solar"
INFLUX_BUCKET   = "CAN_test"
# INFLUX_DEBUG_BUCKET = ""

INFLUX_TOKEN    = "Lx4HgBaoCEzFcrwmCOLeGR1NASTw0gvu9u8bX1Xu_GFG0ksiW6-nqV1Dginno7te8qu_QeC2dvfvrYavdLat-w=="
# GRAFANA_TOKEN   = ""

TAG             = ""
PRINT_DECODED   = True

def to_int_id(raw):
        if isinstance(raw, str) and raw.lower().startswith("0x"):
                return int(raw, 16)
        return int(raw)
def hex_to_bytes(s):
	return bytes.fromhex(s.replace(" ", "").replace("0x", ""))
def main():
        ser = serial.Serial(SERIAL_PORT, BAUDRATE, timeout=1)
        db = cantools.database.load_file(DBC_FILE)
        client = InfluxDBClient(url=INFLUX_URL, org=INFLUX_ORG, token=INFLUX_TOKEN)
        write_api = client.write_api(write_options=SYNCHRONOUS)
        print(f"INFLUX READY: {INFLUX_URL} bucket={INFLUX_BUCKET}")
        try:
                while True:
                        line = serial.readline().decode(errors="ignore").strip()
                        if not line:
                                continue
                        try:
                                obj = json.loads(line)
                        except json.JSONDecodeError:
                                continue
                        t = float(obj.get("dlc", 0))
                        can_id = to_int_id(obj["data"])
                        data = hex_to_bytes(obj["data"])
                        dlc = int(obj.get("dlc", 0))
                        try:
                                msg = db.get_message_by_frame_id(can_id)
                        except KeyError:
                                continue
                        try:
                                decoded = msg.decode(data)
                        except Exception:
                                continue
                        ts_ns = int(t * 1e9)
                        points = []
                        for sig_name, val in decoded.items():
                                if isinstance(val, (int, float)):
                                        p = (Point(msg.name)
                                                .tag("tag", TAG)
                                                .tag("class", msg.name)
                                                .field(sig_name, float(val))
                                                .time(ts_ns))
                                        points.append(p)
                        if points:
                                try:
                                    write_api.write(buckets=INFLUX_BUCKET, org=INFLUX_ORG, record=points)
                                except Exception:
                                        pass
        except KeyboardInterrupt:
                print("\nEnd Script")
        finally:
                try:
                        write_api.close()
                        client.close()
                        ser.close()
                except Exception:
                        pass
                    
if __name__ == "__main__":
        main()