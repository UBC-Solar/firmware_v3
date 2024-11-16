# Script to send a single CAN message

import can


CAN_ID              = 0x777
CAN_DATA            = [0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08]
CAN_IS_EXTENDED     = False

bus = can.interface.Bus(channel='can0', bustype='socketcan')
message = can.Message(arbitration_id=CAN_ID, data=CAN_DATA, is_extended_id=CAN_IS_EXTENDED)
try:
        bus.send(message)
        print(f"SENT")
except can.CanError as e:
    print(f"Message NOT sent {e}")
