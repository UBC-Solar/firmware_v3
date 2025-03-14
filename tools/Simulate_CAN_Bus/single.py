# Script to send a single CAN message

import can


CAN_ID              = 0x622
CAN_DATA            = [0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]
CAN_IS_EXTENDED     = False

bus = can.interface.Bus(channel='can0', bustype='socketcan')
message = can.Message(arbitration_id=CAN_ID, data=CAN_DATA, is_extended_id=CAN_IS_EXTENDED)
try:
        bus.send(message)
        print(f"SENT")
except can.CanError as e:
    print(f"Message NOT sent {e}")
