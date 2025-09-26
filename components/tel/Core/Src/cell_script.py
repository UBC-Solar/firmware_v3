import re, time, serial
from influxdb_client import InfluxDBClient, Point
from influxdb_client.client.write_api import SYNCHRONOUS

SERIAL_PORT   = "/dev/ttyUSB0"
BAUDRATE      = 115200
INFLUX_URL    = "http://100.120.214.69"
INFLUX_ORG    = "UBC Solar"
INFLUX_BUCKET = "CAN_test"
INFLUX_TOKEN  = ""
HEX_RE = re.compile(r'^[0-9A-Fa-f]+$')
def _latin1(b: bytes) -> str:
    # Your CAN class takes a latin-1 string that preserves raw bytes 0x00..0xFF 1:1
    return b.decode('latin-1', errors='ignore')
def parse_brightside_hex_to_can_message_str(line: str) -> str | None:
    """
    Accepts a raw line like:
      41cc36a20c78937523000007537b1448c200000c43080d0a
    Returns a latin-1 string of the first 21 bytes:
      [8B ts][0x23][4B id][8B data]
    (DLC, CR, LF are ignored if present.)
    """
    # keep only hex
    s = re.sub(r'[^0-9A-Fa-f]', '', line)
    if len(s) % 2 != 0 or not s:
        return None
    try:
        raw = bytes.fromhex(s)
    except ValueError:
        return None
    # The minimal frame we can use is 8+1+4+8 = 21 bytes
    if len(raw) < 21:
        return None
    # Some sources append DLC (1B) then CRLF; we only need first 21
    msg = raw[:21]
    # Optional sanity check: middle separator byte
    sep = msg[8]
    if sep != 0x23:  # '#'
        # Not fatal; many builds still use same layout, but you can enforce if you want:
        # return None
        pass
    return _latin1(msg)
def write_can_to_influx(write_api, can_obj):
    """
    can_obj is your CAN(message_str). It exposes can_obj.data with:
      Source[], Class[], Measurement[], Value[], Timestamp[]
    We write each numeric (int/float) Value as a field on measurement 'Class'.
    """
    data = can_obj.data
    if not data or not data.get("Measurement"):
        return
    src = (data["Source"][0] if data["Source"] else "UNKNOWN")
    cls = (data["Class"][0] if data["Class"] else "CAN")
    points = []
    for name, val, ts in zip(data["Measurement"], data["Value"], data["Timestamp"]):
        if isinstance(val, (int, float)):
            points.append(
                Point(cls)
                .tag("source", src)
                .tag("measurement", name)  # optional: makes Grafana filtering easy
                .field(name, float(val))
                .time(int(ts * 1e9))      # ns
            )
    if points:
        write_api.write(bucket=INFLUX_BUCKET, org=INFLUX_ORG, record=points)
def handle_raw_hex_line(line: str, write_api):
    """
    Parse one raw hex line, feed your CAN class, write to Influx.
    """
    msg_str = parse_brightside_hex_to_can_message_str(line)
    if not msg_str:
        return False
    try:
        can_obj = CAN(msg_str)  # your class does DBC decoding internally
        # Logging for visibility
        cls = can_obj.data["Class"][0] if can_obj.data["Class"] else "CAN"
        print(f"[DECODE] {cls} {can_obj.data['display_data']['COL']['Hex_ID'][0]} "
              f"-> {dict(zip(can_obj.data['Measurement'], can_obj.data['Value']))}")
        write_can_to_influx(write_api, can_obj)
        return True
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
    main_loop()