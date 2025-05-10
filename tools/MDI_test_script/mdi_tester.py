import can
import struct
import tkinter as tk
from tkinter import ttk
import threading
import time

# Configure the PCAN interface (modify channel and bustype as needed)
CAN_CHANNEL = 'PCAN_USBBUS1'  # Change this according to your setup
BUS_TYPE = 'pcan'  # Use 'socketcan' for Linux, 'pcan' for Peak-CAN on Windows

class CAN_GUI:
    def __init__(self, root):
        self.root = root
        self.root.title("PCAN Message Sender")

        self.running = True  # Flag to control the send thread

        # Initialize CAN bus
        try:
            self.bus = can.interface.Bus(channel=CAN_CHANNEL, bustype=BUS_TYPE, bitrate=500000)
        except Exception as e:
            print(f"Error initializing CAN bus: {e}")

        # Throttle slider
        self.throttle_label = tk.Label(root, text="Throttle")
        self.throttle_label.pack()
        self.throttle = tk.IntVar()
        self.throttle_slider = ttk.Scale(root, from_=0, to=1023, orient=tk.HORIZONTAL, variable=self.throttle)
        self.throttle_slider.pack()

        # Regen slider
        self.regen_label = tk.Label(root, text="Regen")
        self.regen_label.pack()
        self.regen = tk.IntVar()
        self.regen_slider = ttk.Scale(root, from_=0, to=1023, orient=tk.HORIZONTAL, variable=self.regen)
        self.regen_slider.pack()

        # Direction checkbox
        self.direction = tk.BooleanVar()
        self.direction_checkbox = tk.Checkbutton(root, text="Direction", variable=self.direction)
        self.direction_checkbox.pack()

        # Power mode checkbox
        self.power_mode = tk.BooleanVar()
        self.power_mode_checkbox = tk.Checkbutton(root, text="Power Mode", variable=self.power_mode)
        self.power_mode_checkbox.pack()

        # Start the CAN send thread
        self.send_thread = threading.Thread(target=self.send_loop, daemon=True)
        self.send_thread.start()

        # Ensure thread stops on window close
        self.root.protocol("WM_DELETE_WINDOW", self.on_close)

    def send_can(self):
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

    def send_loop(self):
        while self.running:
            self.send_can()
            time.sleep(0.05)  # 50 milliseconds

    def on_close(self):
        self.running = False
        time.sleep(0.05)  # Let thread finish cleanly
        self.root.destroy()

if __name__ == "__main__":
    root = tk.Tk()
    app = CAN_GUI(root)
    root.mainloop()