# BMS GUI Application

This application is dedicated specifically for our team. It takes the voltage, temperature, SOC, and status data for all modules from Brightside's BMS and presents them in a readable interface.

## To run the program:

1. Make sure to clone all files into your computer
2. Install the required libraries (shown in the requirements.txt file) from pip
3. Run "run_me.py" through an ide (I use PyCharm) to start the program

If you want to use the simulation file with random numbers:

1. Install a COM emulator program; [com0com](https://sourceforge.net/projects/com0com/) (Null-Modem Emulator) is free to use
2. After emulating, run the "simulate_data_entry.py"
3. Now you can select and read the port in the UI
