# Main Control Board (Brightside)

Welcome to Brightside's main control board codebase!

## Overview

The function of Brightside's main control board (MCB) is to take driver inputs from the steering wheel, dashboard, and pedal, and then produce the appropriate outputs. These outputs are typically in the form of CAN messages. Often, these CAN messages are read by Brightside's Mitsuba motor controller - we are using the Tritium motor controller as a backup.

Some examples of the kind of functionality the MCB provides (not exhaustive) are provided below:

  - When the driver presses the accelerator pedal, the MCB reads the pedal encoder and outputs a CAN message onto the CAN bus that is read by the motor controller which generates    the correct amount of torque depending on the pedal position.

  - When the driver enables cruise control by pressing the `CRUISE_UP` button, the MCB switches to the `CRUISE_READY` state and sends the appropriate velocity-control CAN message    onto the bus which is again read by the motor controller.

  - The MCB constantly monitors motor temperature and if it gets too high, the MCB will go into the `MOTOR_SHUTOFF` state and stop sending motor control CAN messages.

## Hardware

The microprocessor that this code is designed for is the ARM-based STM32F103RCT6. The exact PCB schematic for the MCB can be found on the UBC Solar Altium 365 workspace. You will need to create an Altium account and get added to the workspace to be able to access the schematics.

## Changes from Firmware-v2

- Firmware now uses FreeRTOS for improved time determinism and real-time performance
- Interrupts have been configured for most if not all button and switch inputs
- Bare-metal register write/reads have been replaced by HAL library API calls
- New features (regen braking, cruise control)
- Abstraction and Software components implemented

### Implementation of RTOS
- RTOS allows for an easy way of separating code that needs to run at different speeds
- Different tasks:
  - Handling the state machine.
  - Handling CAN inputs.
  - Heartbeat/LED flashing
this is a non-exhaustive list.

### Regenerative braking 
- One-pedal driving.
- Regen is braking is dynamically enabled/disabled depending on inputs from BMS, MDU or the switches on the MCB.

### Cruise control
- Driver can enable cruise control by pressing a button on the steering wheel which sets the cruise speed to the velocity of the car.
- Cruise control can be increase/decreased by pressing buttons on the steering wheel.
- Cruise control is automatically disabled on brake press.

### Abstraction and SWCs
- Abstraction increases readability and maintainability of code.
- Implemented complex system level code behind simplified interface.
- Modular pieces of software that encapsulate specific functionality are separated into .c & .h files.
No function is greater than 1.5 pages long.






