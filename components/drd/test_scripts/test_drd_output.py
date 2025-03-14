import can

CAN_ID = 0x401  # The ID to filter and extract data from

def receive_can_message():
    """Receives CAN messages and extracts data if the ID matches 0x401."""
    bus = can.interface.Bus(channel='can0', bustype='socketcan')

    print("Listening for CAN messages...")

    while True:
        message = bus.recv()  # Receive a message from the CAN bus

        if message is None:
            continue  # Ignore if no message was received

        if message.arbitration_id == CAN_ID and len(message.data) >= 5:
            accel_adc = (message.data[0] << 8) | message.data[1]
            regen_adc = (message.data[2] << 8) | message.data[3]
            flags = message.data[4]

            mech_brake_pressed = (flags & 0x01) != 0
            drive_mode = (flags & 0x02) != 0

            print(f"Received CAN message with ID {hex(CAN_ID)}")
            print(f"  Accel ADC: {accel_adc}")
            print(f"  Regen ADC: {regen_adc}")
            print(f"  Mech Brake Pressed: {mech_brake_pressed}")
            print(f"  Drive Mode: {drive_mode}")
        else:
            print(f"Received message with ID {hex(message.arbitration_id)}, ignoring.")

if __name__ == "__main__":
    receive_can_message()
