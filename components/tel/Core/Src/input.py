import serial

ser = serial.Serial('/dev/cu.usbserial-2110', 115200)

while True:
    print(ser.readline().decode('latin-1'))