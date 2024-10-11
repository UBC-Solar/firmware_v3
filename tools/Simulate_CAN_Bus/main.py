import can
import time
import yaml
import signal
import threading
from pathlib import Path

# ANSI color for yellow
ANSI_YELLOW = "\033[33m"
ANSI_RESET = "\033[0m"

RATE_SCALER = 4
MSG_COUNT_IDX = 3

global can_messages 
can_messages = {}

def load_can_messages(filename):
    # Use Path to handle the file path
    path = Path(__file__).parent / filename
    with open(path, 'r') as file:
        messages = yaml.safe_load(file)
    
    for msg in messages:
        # Keep ID as a string
        can_id = str(msg['ID'])
        # Convert interval to seconds
        interval = msg['interval'] / 1000.0
        # Convert hex string data to a byte list
        data = [int(msg['data'][i:i+2], 16) for i in range(0, len(msg['data']), 2)]
        dlc = msg['dlc']  # Data Length Code

        count = 0
        can_messages[can_id] = [interval, data, dlc, count]

# Signal handler for graceful shutdown
def signal_handler(sig, frame):
    print(ANSI_YELLOW + "\nExiting... Here are the message counts:" + ANSI_RESET)
    for can_id, message_data in can_messages.items():
        count = message_data[MSG_COUNT_IDX]
        print(ANSI_YELLOW  + f"ID: {can_id}, Count: {count}" + ANSI_RESET)
    exit(0)

# Function to send a specific CAN message
def send_message(bus, can_id, data, rate, dlc):
    is_extended = False if len(can_id) <= 3 else True

    message = can.Message(arbitration_id=int(can_id, 16), data=data[:dlc], is_extended_id=is_extended)
    while True:
        try:
            bus.send(message)
            can_messages[can_id][MSG_COUNT_IDX] += 1
            print(f"ID: {can_id}, Count: {can_messages[can_id][MSG_COUNT_IDX]}")
        except can.CanError as e:
            print(f"Message NOT sent {e}")
        time.sleep(rate * RATE_SCALER)

def send_can_messages():
    bus = can.interface.Bus(channel='can0', bustype='socketcan')
    load_can_messages('can_messages.yaml')

    threads = []
    for can_id, [interval, data, dlc, count] in can_messages.items():
        thread = threading.Thread(target=send_message, args=(bus, can_id, data, interval, dlc))
        thread.start()
        threads.append(thread)
    
    for thread in threads:
        thread.join()

if __name__ == "__main__":
    # Set up signal handler
    signal.signal(signal.SIGQUIT, signal_handler)
    signal.signal(signal.SIGINT, signal_handler)
    send_can_messages()
