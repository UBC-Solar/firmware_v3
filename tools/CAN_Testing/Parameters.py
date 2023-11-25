# File name parameters
INFLUX_FILE_NAME = "influx_log.txt"     # Name of the generated InfluxDB format log file

# Jitter analyzer parameters
INTERVAL_MS                         = 1000                          # 1 ms for "exampleLog.txt", normally 1000 
MAX_CAN_LOAD_BYTES_PER_S            = 500 * pow(2, 10) / 8          # 500 kilo-bit/s = 64000 bytes
MAX_CAN_LOAD_INTERVAL_BYTES         = MAX_CAN_LOAD_BYTES_PER_S * (INTERVAL_MS / 1000)
T_BIT                               = 1.0 / (1000000 * 8)           # UNUSED: Time for 1 bit to travel

# Recieve.py parameters
DELTA = 100  # +- 100ms regardless of interval. Ex. interval = 200ms so 100-300ms is acceptable

