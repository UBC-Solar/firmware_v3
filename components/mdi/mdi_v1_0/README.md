# Motor Driver Interface (Brightside)

Welcome to Brightside's motor driver interface codebase!

## Overview

The function of Brightside's motor driver interface (MDI) is to act as an interface between the MCB and the Mitsuba Motor Controller. The MDI takes messages from the MCB and drives the controller by sending the appropriate analog and digital signals the controller expects. It also takes the telemetry data sent by the motor controller and sends it back to the MCB. 

Some examples of the kind of functionality the MDI provides (not exhaustive) are provided below:

  - Communicate with the motor controller the current state of the car (Going Forward, Going in Reverse, Regen). 
  - Send the analog signals that drive the motor controller to the acceleration desired based on the inputs from the MCB.
  - Receive the CAN messesages and HALL sensor output data from the motor controller, parse it and send it back to the rest of the car.

## Hardware

The microprocessor that this code is designed for is the ARM-based STM32F103RCT6. The exact PCB schematic for the MCB can be found on the UBC Solar Altium 365 workspace. You will need to create an Altium account and get added to the workspace to be able to access the schematics.

## Software 

The main CAN struct used to hold the CAN data and the functions used to receive, parse and send CAN messages are found in Inc/can.h and Src/can.c respectively. Src/MDI_Helper_Functions.c contains helper functions used in the main function. 






