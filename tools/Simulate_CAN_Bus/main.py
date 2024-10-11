import can
import time
import yaml
from threading import Thread
from pathlib import Path

def load_can_messages(filename):
    # Use Path to handle the file path
    path = Path(__file__).parent / filename
    with open(path, 'r') as file:
        messages = yaml.safe_load(file)
    
    can_messages = {}
    for msg in messages:
        # Keep ID as a string
        can_id = str(msg['ID'])
        # Convert interval to seconds
        interval = msg['interval'] / 1000.0
        # Convert hex string data to a byte list
        data = [int(msg['data'][i:i+2], 16) for i in range(0, len(msg['data']), 2)]
        dlc = msg['dlc']  # Data Length Code
        can_messages[can_id] = (interval, data, dlc)
    
    return can_messages

# Function to send a specific CAN message
def send_message(bus, can_id, data, rate, dlc):
    message = can.Message(arbitration_id=int(can_id, 16), data=data[:dlc], is_extended_id=False)
    while True:
        try:
            bus.send(message)
            print(f"Message sent: ID: {can_id}, Data: {message.data.hex()}, DLC: {dlc}")
        except can.CanError as e:
            print(f"Message NOT sent {e}")
        time.sleep(rate)

def send_can_messages():
    bus = can.interface.Bus(channel='can0', bustype='socketcan')
    can_messages = load_can_messages('can_messages.yaml')

    threads = []
    for can_id, (interval, data, dlc) in can_messages.items():
        thread = Thread(target=send_message, args=(bus, can_id, data, interval, dlc))
        thread.start()
        threads.append(thread)
    
    for thread in threads:
        thread.join()

if __name__ == "__main__":
    send_can_messages()
