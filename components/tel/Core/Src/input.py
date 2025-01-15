import serial

ser = serial.Serial('/dev/cu.usbserial-130', 115200)

while True:
    print(ser.readline().decode('latin-1'))