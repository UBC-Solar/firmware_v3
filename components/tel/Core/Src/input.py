import serial
import time

# Configure the COM port
ser = serial.Serial(
   port='/dev/cu.usbserial-1130',       # COM port (e.g., COM1, COM3, or /dev/ttyUSB0 for Linux)
   baudrate=115200,     # Baud rate (match with the device's settings)
)

time.sleep(2)

try:
    while True:
        s = ser.readline()
        print(s.decode('latin-1'))

except KeyboardInterrupt:
    print("\nProgram interrupted by user. Exiting...")
except serial.SerialException as e:
    print(f"Serial error: {e}")
finally:
    ser.close()