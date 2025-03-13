import can
import struct
import tkinter as tk
from tkinter import ttk

# Configure the PCAN interface (modify channel and bustype as needed)
CAN_CHANNEL = 'PCAN_USBBUS1'  # Change this according to your setup
BUS_TYPE = 'pcan'  # Use 'socketcan' for Linux, 'pcan' for Peak-CAN on Windows

class CAN_GUI:
    def __init__(self, root):
        self.root = root
        self.root.title("PCAN Message Sender")

        # Initialize CAN bus
        try:
            self.bus = can.interface.Bus(channel=CAN_CHANNEL, bustype=BUS_TYPE, bitrate=500000)
        except Exception as e:
            print(f"Error initializing CAN bus: {e}")
            return

        # Throttle slider
        self.throttle_label = tk.Label(root, text="Throttle")
        self.throttle_label.pack()
        self.throttle = tk.IntVar()
        self.throttle_slider = ttk.Scale(root, from_=0, to=1023, orient=tk.HORIZONTAL, variable=self.throttle, command=self.send_can)
        self.throttle_slider.pack()

        # Regen slider
        self.regen_label = tk.Label(root, text="Regen")
        self.regen_label.pack()
        self.regen = tk.IntVar()
        self.regen_slider = ttk.Scale(root, from_=0, to=1023, orient=tk.HORIZONTAL, variable=self.regen, command=self.send_can)
        self.regen_slider.pack()

        # Direction checkbox
        self.direction = tk.BooleanVar()
        self.direction_checkbox = tk.Checkbutton(root, text="Direction", variable=self.direction, command=self.send_can)
        self.direction_checkbox.pack()

        # Power mode checkbox
        self.power_mode = tk.BooleanVar()
        self.power_mode_checkbox = tk.Checkbutton(root, text="Power Mode", variable=self.power_mode, command=self.send_can)
        self.power_mode_checkbox.pack()

    def send_can(self, event=None):
        throttle_value = self.throttle.get() & 0x3FF  # 10-bit value
        regen_value = self.regen.get() & 0x3FF  # 10-bit value

        # Pack the values into bytes
        throttle_bytes = struct.pack('<H', throttle_value)  # Little-endian 16-bit
        regen_bytes = struct.pack('<H', regen_value)  # Little-endian 16-bit

        # Pack direction and power mode into a single byte
        control_byte = (int(self.direction.get()) << 1) | int(self.power_mode.get())

        # Construct the CAN message (8 bytes total)
        data = bytearray(8)
        data[0:2] = throttle_bytes
        data[2:4] = regen_bytes
        data[4] = control_byte

        message = can.Message(arbitration_id=0x123, data=data, is_extended_id=False)

        try:
            self.bus.send(message)
            print(f"Sent CAN message: {message}")
        except can.CanError as e:
            print(f"Failed to send CAN message: {e}")

if __name__ == "__main__":
    root = tk.Tk()
    app = CAN_GUI(root)
    root.mainloop()