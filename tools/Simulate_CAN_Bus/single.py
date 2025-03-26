# Script to send a single CAN message
import time
import can


PARK = 0
REVERSE = 1
FORWARD = 2

CAN_ID_2              = 0x500
CAN_DATA_2            = [REVERSE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]
# CAN_DATA_2            = [PARK, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]
# CAN_DATA_2            = [FORWARD, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]
CAN_IS_EXTENDED_2     = False

# bus = can.interface.Bus(channel='can0', bustype='socketcan')
# message = can.Message(arbitration_id=CAN_ID_2, data=CAN_DATA_2, is_extended_id=CAN_IS_EXTENDED_2)
# try:
#         bus.send(message)
#         print(f"SENT")
# except can.CanError as e:
#     print(f"Message NOT sent {e}")


CAN_ID              = 0x08850225
CAN_DATA            = [0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]
CAN_IS_EXTENDED     = True

value = 0  

# bus = can.interface.Bus(channel='can0', bustype='socketcan')
# message = can.Message(arbitration_id=CAN_ID, data=CAN_DATA, is_extended_id=CAN_IS_EXTENDED)
# try:
#         bus.send(message)
#         print(f"SENT {2 * 3.14159265 * 3.6 * value * 0.283 / (60)}")
# except can.CanError as e:
#     print(f"Message NOT sent {e}")
    
for i in range(500):
    value += 1

    # Set bits 35-46
    CAN_DATA[4] = (CAN_DATA[4] & 0x07) | ((value & 0x1F) << 3)  # Preserve lower 3 bits, set bits 35-39
    CAN_DATA[5] = (value >> 5) & 0xFF                           # Set bits 40-47
    CAN_DATA[6] = (CAN_DATA[6] & 0xFE) | ((value >> 11) & 0x01)  # Preserve other bits, set bit 46


    bus = can.interface.Bus(channel='can0', bustype='socketcan')
    message = can.Message(arbitration_id=CAN_ID, data=CAN_DATA, is_extended_id=CAN_IS_EXTENDED)
    try:
            bus.send(message)
            print(f"SENT {2 * 3.14159265 * 3.6 * value * 0.283 / (60)}")
    except can.CanError as e:
        print(f"Message NOT sent {e}")

    CAN_DATA_2[0] = value % 3
    message = can.Message(arbitration_id=CAN_ID_2, data=CAN_DATA_2, is_extended_id=CAN_IS_EXTENDED_2)
    try:
            bus.send(message)
            print(f"SENT {CAN_DATA_2[0]}")
    except can.CanError as e:
        print(f"Message NOT sent {e}")

    time.sleep(0.050)




# CAN_ID_3              = 0x580
# CAN_DATA_3            = [0x0]
# CAN_DATA_3            = [PARK, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]
# CAN_DATA_3            = [FORWARD, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]
# CAN_IS_EXTENDED_3     = False

# bus = can.interface.Bus(channel='can0', bustype='socketcan')
# message = can.Message(arbitration_id=CAN_ID_3, data=CAN_DATA_3, dlc=1, is_extended_id=CAN_IS_EXTENDED_3)
# try:
#         bus.send(message)
#         print(f"SENT")
# except can.CanError as e:
#     print(f"Message NOT sent {e}")