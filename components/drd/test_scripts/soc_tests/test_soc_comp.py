# To get SOC test to imitate comp you need to get the comp data first. 
# Get this data here https://drive.google.com/drive/u/2/folders/1MyjOjVC0uGB84lE8OuudrDIxAQNEUokJ and put it in comp_data folder

import csv
import struct
import time
import threading
import can
import os

CURRENT = 0
VOLTAGE = 1

CURRENT_CSV_FILE = "comp_data/2024_comp_current.csv"
VOLTAGE_CSV_FILE = "comp_data/2024_comp_voltage.csv"

def scale_and_pack(value, mode):
    if mode == CURRENT:
        scaled = int(65.535 * value)
        packed = struct.pack('<h', scaled)  # int16_t little-endian
        return bytearray(packed + bytes(5))
    elif mode == VOLTAGE:
        scaled = int(140 * value)
        packed = struct.pack('<H', scaled)  # uint16_t little-endian
        return bytearray(packed + bytes(4))
    else:
        raise ValueError("Invalid mode")

def read_values_from_csv(file_path):
    with open(file_path, newline='') as csvfile:
        reader = csv.reader(csvfile)
        next(reader)  # skip header
        return [float(row[1]) for row in reader]

def send_can_messages(values, mode, can_id, delay):
    bus = can.interface.Bus(channel='can0', bustype='socketcan')
    time.sleep(delay)

    for value in values:
        data = scale_and_pack(value, mode)
        msg = can.Message(arbitration_id=can_id, data=data, is_extended_id=False)
        try:
            bus.send(msg)
            print(f"Sent on {hex(can_id)}: {list(data)}")
        except can.CanError:
            print(f"Failed to send message on {hex(can_id)}")
        time.sleep(0.1)

def main():
    # Get the absolute path to the directory where the current Python file is located
    base_path = os.path.dirname(os.path.abspath(__file__))

    current_values = read_values_from_csv(base_path + "/" + CURRENT_CSV_FILE)
    votlage_values = read_values_from_csv(base_path + "/" + VOLTAGE_CSV_FILE)

    current_thread = threading.Thread(target=send_can_messages, args=(current_values, CURRENT, 0x450, 0))
    voltage_thread = threading.Thread(target=send_can_messages, args=(votlage_values, VOLTAGE, 0x623, 0.02))

    current_thread.start()
    voltage_thread.start()

    current_thread.join()
    voltage_thread.join()

if __name__ == '__main__':
    main()
