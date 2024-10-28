import can

def send_can_message():
    # Set up the CAN bus
    bus = can.interface.Bus(channel='can0', bustype='socketcan')  # Change 'can0' to your actual channel

    # Create a CAN message
    message = can.Message(arbitration_id=0x696, data=[0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00], is_extended_id=False)

    # Send the message
    try:
        bus.send(message)
        print("Message sent successfully!")
    except can.CanError:
        print("Message failed to send.")

if __name__ == "__main__":
    send_can_message()
