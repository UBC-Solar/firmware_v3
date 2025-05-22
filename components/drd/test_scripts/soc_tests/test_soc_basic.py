# Usage 

# First send the turn signal left signal. 

# Turn on Power mode for driving

# send SoC message. Start at 100% SoC, go down to 0% over 30 seconds . Start a thread for this

# send PackCurrent signal and the PackVoltage signal. Start at 34A and 139V. then over 30 seconds go down to -20A and 60V. Start a thread for this

# send the turn signal right signal. 

# Send drive state Drive 0x500,   0 = PARK, 1 = REVERSE, 2 = FORWARD

# Send MotorRotatingSpeed value of 1. Increase to 500 over a period of 10 Seconds. Make a thread for this so it can keep happening

# Send drive state Park 0x500,   0 = PARK, 1 = REVERSE, 2 = FORWARD

# Send fault lights in a pattern. One-byone and then all off and then all on and then all off.

# Send drive state Reverse 0x500,   0 = PARK, 1 = REVERSE, 2 = FORWARD

# Once the MotorRotatingSpeed thread finishs, clean it up and now go from 500 to value of 1 in 5 seconds. Dont make a thred

# Send drive state Park 0x500,   0 = PARK, 1 = REVERSE, 2 = FORWARD

# Send drive state Reverse 0x500,   0 = PARK, 1 = REVERSE, 2 = FORWARD


import can
import cantools
import sys
import threading
from typing import Optional
import time
import os
import time
import threading
from pprint import pprint


""" CHANGE BASED ON YOUR COMPUTER """
WINDOWS = False


script_dir = os.path.dirname(os.path.abspath(sys.argv[0]))
DBC_FILE = os.path.join(script_dir, '../brightside.dbc')
FAULT       = 1
CLEAR       = 0
RIGHT   = 2
LEFT   = 1
OFF = 0
PARK = 0
REVERSE = 1
FORWARD = 2
POWER = 0
ECO = 1

# Load DBC file (replace 'your_dbc.dbc' with your actual DBC file path)
try:
    db = cantools.db.load_file(DBC_FILE)
except Exception as e:
    print(f"Failed to load DBC file: {e}")
    sys.exit(1)

class CANMessage:
    """Represents a CAN message with signals defined in the DBC."""
    def __init__(self, message_identifier, db):
        self.db = db
        if isinstance(message_identifier, int):
            self.message = self.db.get_message_by_frame_id(message_identifier)
        else:
            self.message = self.db.get_message_by_name(message_identifier)
        if not self.message:
            raise ValueError(f"Message {message_identifier} not found in DBC")
        self.msg_id = self.message.frame_id
        self.is_extended = self.message.is_extended_frame
        self.signal_values = {sig.name: sig.initial or 0 for sig in self.message.signals}

    def set_data(self, signal_name: str, value):
        """Set the value of a specific signal in the message."""
        if signal_name not in self.signal_values:
            raise KeyError(f"Signal '{signal_name}' not found in message {self.message.name}")
        self.signal_values[signal_name] = value

    def encode(self) -> bytes:
        """Encode the current signal values into CAN data bytes."""
        return self.message.encode(self.signal_values)

class CANBus:
    """Handles CAN bus communication."""
    def __init__(self, bustype: str, channel: str):
        self.bustype = bustype
        self.channel = channel
        self.bus = None
        self._connect()

    def _connect(self):
        """Establish connection to the CAN bus."""
        try:
            self.bus = can.interface.Bus(bustype=self.bustype, channel=self.channel, bitrate=500000)
        except Exception as e:
            print(f"CAN init error: {e}")
            sys.exit(1)

    def send(self, msg: CANMessage):
        """Send a CAN message."""
        data = msg.encode()
        can_msg = can.Message(
            arbitration_id=msg.msg_id,
            data=data,
            is_extended_id=msg.is_extended
        )
        try:
            self.bus.send(can_msg)
            time.sleep(0.01)
        except can.CanError as e:
            print(f"Error sending message: {e}")

    def start_receiver(self, callback):
        """Start a thread to receive and decode CAN messages."""
        def receive_loop():
            while True:
                msg = self.bus.recv()
                if msg:
                    try:
                        msg.data += b'\x00' * (8 - len(msg.data))
                        decoded = db.decode_message(msg.arbitration_id, msg.data)
                        callback(msg, decoded)
                    except Exception as e:
                        print(f"No Parse for {hex(msg.arbitration_id)}: {e}")
        threading.Thread(target=receive_loop, daemon=True).start()

    # Pre-defined message sender methods
    def send_bms_comm_fault(self, fault: bool):
        msg = CANMessage(1570, db)
        msg.set_data('SlaveBoardComm', fault)
        self.send(msg)


    def send_pack_voltage(self, value: int):
        msg = CANMessage(1571, db)
        msg.set_data('TotalPackVoltage', value)
        self.send(msg)

    def send_batt(self, value: int):
        self.send_pack_voltage(value)

    def send_batt_hi(self, fault: bool):
        self.send_pack_voltage(139 if fault else 100)

    def send_batt_lo(self, fault: bool):
        self.send_pack_voltage(80 if fault else 100)

    def send_mtr_oc(self, fault: bool):
        msg = CANMessage(0x08A50225, db)
        msg.set_data('OverCurrentError', fault)
        self.send(msg)
        
    def send_mtr_flt(self, fault: bool):
        msg = CANMessage(0x08A50225, db)
        msg.set_data('MotorSystemError', fault)
        self.send(msg)
        
    def send_mtr_ot(self, fault: bool):
        msg = CANMessage(0x08A50225, db)
        msg.set_data('FETThermistorError', fault)
        self.send(msg)

    def send_pack_current(self, value: int):    
        msg = CANMessage(1104, db)
        msg.set_data('PackCurrent', value)
        self.send(msg)

    def send_charge_oc(self, fault: bool):
        self.send_pack_current(2)

        msg = CANMessage(1570, db)
        msg.set_data('ChargeOvercurrent', fault)
        self.send(msg)
        
    def send_discharge_oc(self, fault: bool):
        self.send_pack_current(-2)

        msg = CANMessage(1570, db)
        msg.set_data('ChargeOvercurrent', fault)
        self.send(msg)
        
    def send_batt_ov(self, fault: bool):
        msg = CANMessage(1570, db)
        msg.set_data('OverVoltage', fault)
        self.send(msg)

    def send_all_fault(self):
        print("CHECK DRD LED FOR BATT_LO")
        self.send_batt_lo(FAULT)
        self.send_pack_current(2)
        print("CHECK DRD LED FOR DCH_OC")
        self.send_faults(FAULT)
        time.sleep(1)
        print("CHECK DRD LED FOR CH_OC")
        self.send_pack_current(-2)
        self.send_faults(FAULT)

        print("CHECK DRD LED FOR BATT_HI")
        self.send_batt_hi(FAULT)
        print("FAULTS DONE")


    def send_clear_faults(self):
        self.send_faults(CLEAR)
        self.send_batt_lo(CLEAR)
        print("CLEARING DONE")

    def send_faults(self, fault: bool):
        msg = CANMessage(1570, db)
        msg.set_data('SlaveBoardComm', fault)
        msg.set_data('ChargeOvercurrent', fault)
        msg.set_data('OverVoltage', fault)
        msg.set_data('UnderVoltage', fault)
        msg.set_data('OverTemp', fault)
        msg.set_data('BMSSelfTest', fault)
    
        msg2 = CANMessage(1104, db)
        msg2.set_data('ESTOP_Pressed', fault)

        msg3 = CANMessage(0x08A50225, db)
        msg3.set_data('FETThermistorError', fault)
        msg3.set_data('MotorSystemError', fault)
        msg3.set_data('OverCurrentError', fault)

        self.send(msg)
        self.send(msg2)
        self.send(msg3)

    def send_batt_uv(self, fault: bool):
        msg = CANMessage(1570, db)
        msg.set_data('UnderVoltage', fault)
        self.send(msg)

    def send_batt_ot(self, fault: bool):
        msg = CANMessage(1570, db)
        msg.set_data('OverTemp', fault)
        self.send(msg)

    def send_batt_flt(self, fault: bool):
        msg = CANMessage(1570, db)
        msg.set_data('BMSSelfTest', fault)
        self.send(msg)

    def send_estop(self, fault: bool):
        msg = CANMessage(1104, db)
        msg.set_data('ESTOP_Pressed', fault)
        self.send(msg)

    def send_soc(self, soc: int):
        msg = CANMessage(1572, db)
        msg.set_data('SOC', soc)
        self.send(msg)

    def send_turn_signal(self, val: int):
        msg = CANMessage(0x580, db)

        if (val == RIGHT):
            msg.set_data('RTSSIGNAL', 1)
            msg.set_data('LTSSIGNAL', 0)
        elif (val == LEFT):
            msg.set_data('RTSSIGNAL', 0)
            msg.set_data('LTSSIGNAL', 1)
        elif (val == OFF):
            msg.set_data('RTSSIGNAL', 0)
            msg.set_data('LTSSIGNAL', 0)
        
        self.send(msg)

    def send_drive_state(self, val: int):
        msg = CANMessage(0x500, db)
        msg.set_data('DriveState', val)
        self.send(msg)

    def send_drive_mode(self, val: bool):
        msg = CANMessage(0x580, db)
        msg.set_data('DRIVEMODE', val)
        self.send(msg)

    def send_speed_kmh(self, val: int):
        # conver to rpm
        rpm = int((val * 60 / (2 * 3.14159265 * 3.6 * 0.283)) + 1)
        msg = CANMessage(0x08850225, db)
        msg.set_data('MotorRotatingSpeed', rpm)
        self.send(msg)

    def clear_all(self):
        """Reset all signals to 0."""
        # Reset speed to 0
        self.send_speed_kmh(0)

        # Turn off turn signals
        self.send_turn_signal(OFF)

        # Clear all fault lights
        self.send_clear_faults()

        # Reset SoC to 0
        self.send_soc(0)

        # Reset drive mode to ECO (0)
        self.send_drive_mode(ECO)

        # Reset pack current and voltage to 0
        self.send_pack_current(0)
        self.send_pack_voltage(134.4)

        # Reset drive state to PARK (0)
        self.send_drive_state(PARK)

        print("FULL CLEAR DONE")


        


def on_message_received(msg, decoded):
    # Clear the current line
    sys.stdout.write('\r' + ' ' * 100 + '\r')
    sys.stdout.flush()

    # Format and print the decoded message
    formatted_output = f"From DRD:{hex(msg.arbitration_id)}: {{"
    for key, value in decoded.items():
        formatted_output += f"'{key}': {value:>3}, "
    formatted_output = formatted_output.rstrip(', ') + '}'

    sys.stdout.write(formatted_output)
    sys.stdout.flush()


def script_send_soc(can_bus):
    """Send SoC message from 100% to 0% over 30 seconds."""
    for soc in range(100, -1, -1):
        can_bus.send_soc(soc)
        time.sleep(0.3)

def script_send_pack_current_and_voltage(can_bus):
    """Send PackCurrent and PackVoltage signals over 30 seconds."""
    current = 40
    voltage = 134
    current_step = (current - (-20)) /120
    voltage_step = (voltage - 89) / 120

    for _ in range(120):
        can_bus.send_pack_current(current)
        can_bus.send_pack_voltage(voltage)
        current -= current_step
        voltage -= voltage_step
        time.sleep(0.250)

def script_send_speed_kmh(can_bus):
    """Send speed kmh from 1 to 99 over 10 seconds."""
    for speed in range(1, 99):
        can_bus.send_speed_kmh(speed)
        time.sleep(0.1)

def script_toggle_turn_signals(can_bus):
    """Toggle between left turn, right turn, and off every 4 seconds."""
    for speed in range(1, 2):
        can_bus.send_turn_signal(LEFT)
        time.sleep(4)
        can_bus.send_turn_signal(RIGHT)
        time.sleep(4)
        can_bus.send_turn_signal(OFF)
        time.sleep(4)


# Example usage
if __name__ == "__main__":
    # Initialize CAN bus (adjust bustype and channel as needed)

    if WINDOWS:
        can_bus = CANBus(bustype='pcan', channel='PCAN_USBBUS1')
    else:
        can_bus = CANBus(bustype='socketcan', channel='can0')

    can_bus.start_receiver(on_message_received)
    
    can_bus.clear_all()
    time.sleep(1)

    pack_thread = threading.Thread(target=script_send_pack_current_and_voltage, args=(can_bus,))
    pack_thread.start()

    can_bus.clear_all()
    print("DONE TEST")

    # Keep the application running
    try:
        while True:
            pass
    except KeyboardInterrupt:
        sys.exit(0)
