import can
import argparse
import time
import sys

class CANMessage:
    def __init__(self, can_id, data=None, is_extended=False):
        self.id = can_id
        self.data = list(data) if data is not None else [0] * 8  # Default to 8 bytes
        self.is_extended = is_extended

        # Ensure data is at least 8 bytes
        if len(self.data) < 8:
            self.data += [0] * (8 - len(self.data))
        else:
            self.data = self.data[:8]

    # Generic flag and value manipulation methods
    def _set_flag(self, byte_pos, bit_pos, value):
        """Set/Clear a specific bit in the data bytes."""
        if byte_pos >= len(self.data):
            self.data += [0] * (byte_pos - len(self.data) + 1)
        if value:
            self.data[byte_pos] |= (1 << bit_pos)
        else:
            self.data[byte_pos] &= ~(1 << bit_pos)

    def _get_flag(self, byte_pos, bit_pos):
        """Get the value of a specific bit (0 or 1)."""
        if byte_pos >= len(self.data):
            return 0
        return (self.data[byte_pos] >> bit_pos) & 0x01

    def _set_scaled_value(self, byte_start, value, scale, num_bytes=2, little_endian=True):
        """Set an integer value into self.data with specified endianness.

        Args:
            byte_start (int): The starting byte index (0-7).
            value (int): The integer value to encode.
            num_bytes (int): Number of bytes to encode (default 2).
            little_endian (bool): Use little-endian encoding if True, else big-endian.
        """
        if not (0 <= byte_start < 8):
            raise ValueError("byte_start must be between 0 and 7")
        if not (1 <= num_bytes <= 8):
            raise ValueError("num_bytes must be between 1 and 8")
        if byte_start + num_bytes > 8:
            raise ValueError("Value exceeds 8-byte message boundary")

        value = int(value * scale)
        for i in range(num_bytes):
            byte_pos = byte_start + i if little_endian else byte_start + (num_bytes - 1 - i)
            self.data[byte_pos] = (value >> (i * 8)) & 0xFF

    def _get_scaled_value(self, byte_start, scale, num_bytes=2, little_endian=True, signed=False):
        """Get a numerical value with scaling and endianness."""
        value = 0
        for i in range(num_bytes):
            if little_endian:
                byte_pos = byte_start + i
            else:
                byte_pos = byte_start + (num_bytes - 1 - i)
            if byte_pos >= len(self.data):
                byte_val = 0
            else:
                byte_val = self.data[byte_pos]
            value |= byte_val << (i * 8)
        
        if signed and num_bytes == 2:
            if value & 0x8000:
                value -= 0x10000
        elif signed and num_bytes == 4:
            if value & 0x80000000:
                value -= 0x100000000
        return value * scale

    # Setters and Getters for specific faults
    # BMS_COMM_FLT (ID 0x622, byte 0, bit 0)
    def set_bms_comm_fault(self, value):
        self._set_flag(0, 0, value)
    def get_bms_comm_fault(self):
        return self._get_flag(0, 0)

    # BATT_OV (ID 0x622, byte 0, bit 4)
    def set_batt_ov(self, value):
        self._set_flag(0, 4, value)
    def get_batt_ov(self):
        return self._get_flag(0, 4)

    # BATT_FLT (ID 0x622, byte 0, bit 1)
    def set_batt_flt(self, value):
        self._set_flag(0, 1, value)
    def get_batt_flt(self):
        return self._get_flag(0, 1)

    # BATT_OT (ID 0x622, byte 0, bit 2)
    def set_batt_ot(self, value):
        self._set_flag(0, 2, value)
    def get_batt_ot(self):
        return self._get_flag(0, 2)

    # BATT_UV (ID 0x622, byte 0, bit 3)
    def set_batt_uv(self, value):
        self._set_flag(0, 3, value)
    def get_batt_uv(self):
        return self._get_flag(0, 3)

    # DCH/CH_OC Fault (ID 0x622, byte 0, bit 6)
    def set_discharge_charge_oc_fault(self, value):
        self._set_flag(0, 6, value)
    def get_discharge_charge_oc_fault(self):
        return self._get_flag(0, 6)

    # MTR_OC (ID 0x08A50225, byte 2, bit 1)
    def set_mtr_oc(self, value):
        self._set_flag(2, 1, value)
    def get_mtr_oc(self):
        return self._get_flag(2, 1)

    # MTR_FLT (ID 0x08A50225, byte 3, bit 0)
    def set_mtr_flt(self, value):
        self._set_flag(3, 0, value)
    def get_mtr_flt(self):
        return self._get_flag(3, 0)

    # MTR_OT (ID 0x08A50225, byte 0, bit 3)
    def set_mtr_ot(self, value):
        self._set_flag(0, 3, value)
    def get_mtr_ot(self):
        return self._get_flag(0, 3)

    # BATT_HI (ID 0x623, bytes 0-1, scale 0.1)
    def set_batt_voltage(self, voltage):
        self._set_scaled_value(0, voltage, 468)
    def get_batt_voltage(self):
        return self._get_scaled_value(0, 468)

    # ESTOP (ID 0x450, byte 5, bit 5)
    def set_estop_pressed(self, value):
        self._set_flag(5, 5, value)
    def get_estop_pressed(self):
        return self._get_flag(5, 5)

    # Pack Current (ID 0x450, bytes 0-1, scale 0.1, signed)
    def set_pack_current(self, current):
        self._set_scaled_value(0, current, 65.535)
    def get_pack_current(self):
        return self._get_scaled_value(0, 65.535)


FAULT       = 1
CLEAR       = 0
BMS_FAULT_ID   = 0x622
BATT_VOLTAGE_ID     = 0x623
MC_FLAGS_ID     = 0x08A50225
PACK_CURRENT_ID = 0x450

def send_can_msg(bus, id, data, is_extended):
    msg = can.Message(
        arbitration_id=id,
        data=data,
        is_extended_id=is_extended
    )

    try:
        bus.send(msg)
        print(f"Sent {hex(id)}: {bytes(msg.data).hex()}")
    except can.CanError as e:
        print(f"Error sending: {e}")

def send_bms_comm_fault(bus, val):
    msg = CANMessage(BMS_FAULT_ID, is_extended=False)
    msg.set_bms_comm_fault(val)
    send_can_msg(bus, msg.id, msg.data, msg.is_extended)

def send_batt_hi(bus):
    msg = CANMessage(BATT_VOLTAGE_ID, is_extended=False)
    msg.set_batt_voltage(135)
    send_can_msg(bus, msg.id, msg.data, msg.is_extended)

def send_batt_lo(bus):
    msg = CANMessage(BATT_VOLTAGE_ID, is_extended=False)
    msg.set_batt_voltage(85)
    send_can_msg(bus, msg.id, msg.data, msg.is_extended)

def send_batt(bus, val):
    msg = CANMessage(BATT_VOLTAGE_ID, is_extended=False)
    msg.set_batt_voltage(val)
    send_can_msg(bus, msg.id, msg.data, msg.is_extended)

def send_mtr_oc(bus, val):
    msg = CANMessage(MC_FLAGS_ID, is_extended=True)
    msg.set_mtr_oc(val)
    send_can_msg(bus, msg.id, msg.data, msg.is_extended)

def send_mtr_flt(bus, val):
    msg = CANMessage(MC_FLAGS_ID, is_extended=True)
    msg.set_mtr_flt(val)
    send_can_msg(bus, msg.id, msg.data, msg.is_extended)

def send_mtr_ot(bus, val):
    msg = CANMessage(MC_FLAGS_ID, is_extended=True)
    msg.set_mtr_ot(val)
    send_can_msg(bus, msg.id, msg.data, msg.is_extended)

def send_oc(bus, val):
    msg = CANMessage(BMS_FAULT_ID, is_extended=False)
    msg.set_discharge_charge_oc_fault(val)
    send_can_msg(bus, msg.id, msg.data, msg.is_extended)

def send_pack_current(bus, val):
    msg = CANMessage(PACK_CURRENT_ID, is_extended=False)
    msg.set_pack_current(val)
    send_can_msg(bus, msg.id, msg.data, msg.is_extended)

def send_discharge_oc(bus, val):
    send_pack_current(bus, 20)
    time.sleep(0.005)
    send_oc(bus, val)

def send_charge_oc(bus, val):
    send_pack_current(bus, -20)
    time.sleep(0.005)
    send_oc(bus, val)

def send_batt_ov(bus, val):
    msg = CANMessage(BMS_FAULT_ID, is_extended=False)
    msg.set_batt_ov(val)
    send_can_msg(bus, msg.id, msg.data, msg.is_extended)


def send_batt_uv(bus, val):
    msg = CANMessage(BMS_FAULT_ID, is_extended=False)
    msg.set_batt_uv(val)
    send_can_msg(bus, msg.id, msg.data, msg.is_extended)

def send_batt_ot(bus, val):
    msg = CANMessage(BMS_FAULT_ID, is_extended=False)
    msg.set_batt_ot(val)
    send_can_msg(bus, msg.id, msg.data, msg.is_extended)

def send_batt_flt(bus, val):
    msg = CANMessage(BMS_FAULT_ID, is_extended=False)
    msg.set_batt_flt(val)
    send_can_msg(bus, msg.id, msg.data, msg.is_extended)

def send_estop(bus, val):
    msg = CANMessage(PACK_CURRENT_ID, is_extended=False)
    msg.set_estop_pressed(val)
    send_can_msg(bus, msg.id, msg.data, msg.is_extended)

def main():
    WINDOWS = 0         # 0 = Linux, 1 = Windows

    parser = argparse.ArgumentParser(description='Send CAN messages using high-level interface.')
    parser.add_argument('--linux', action='store_true', help='Use Linux socketcan configuration.')
    parser.add_argument('--windows', action='store_true', help='Use Windows PCAN configuration.')
    args = parser.parse_args()

    # if args.linux:
    #     bustype = 'socketcan'
    #     channel = 'can0'
    # elif args.windows:
        # bustype = 'pcan'
        # channel = 'PCAN_USBBUS1'
    # else:
    #     print("Specify --linux or --windows")
    #     sys.exit(1)

    if WINDOWS:
        bustype = 'pcan'
        channel = 'PCAN_USBBUS1'
    else:
        bustype = 'socketcan'
        channel = 'can0'
        
    try:
        bus = can.interface.Bus(channel=channel, bustype=bustype)
    except Exception as e:
        print(f"CAN init error: {e}")
        sys.exit(1)


    # send_bms_comm_fault(bus, FAULT)
    # send_batt_hi(bus)
    # send_batt(bus, 100)
    # send_mtr_oc(bus, FAULT)
    # send_mtr_flt(bus, FAULT)
    # send_mtr_ot(bus, FAULT)
    # send_charge_oc(bus, FAULT)
    # send_discharge_oc(bus, FAULT)
    # send_batt_ov(bus, FAULT)
    # send_batt_uv(bus, FAULT)
    # send_batt_ot(bus, FAULT)
    # send_batt_flt(bus, FAULT)
    send_estop(bus, FAULT)


    bus.shutdown()

if __name__ == "__main__":
    main()