# Driver Information Display (DID)

This contains all firmware code for the DID ECU for Brightside.

## Overview
The driver information display is an ECU responsible for displaying important data from our car onto an LCD screen and fault LEDs. The data comes from other ECUs on the car via CAN bus.  
The driver can switch pages by pressing the next screen button on the MCB.  

## Pages  
### Page 0
- Vehicle Velocity
- Drive state
- Battery SOC
- Simulation Speed

### Page 1
- Low/high voltage warnings
- Low/high temperature warnings

### Page 2
- Motor current
- Solar array current

### Page 3 
- Battery pack voltage/temperature
