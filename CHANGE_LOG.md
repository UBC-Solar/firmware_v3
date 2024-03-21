# Change Log: firmware_v3

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/)

Each version should:
* List its release date in the above format.
* Group changes to describe their impact on the project, as follows:
* `Added` for new features.
* `Changed` for changes in existing functionality.
* `Deprecated` for once-stable features removed in upcoming releases.
* `Removed` for deprecated features removed in this release.
* `Fixed` for any bug fixes.

<br>

## Main Control Board (MCB)

### MCB v1.0
Added
* Regenerative braking
  * Enables [one-pedal driving](https://www.energy.gov/energysaver/one-pedal-driving-explained#:~:text=What%20is%20One%2DPedal%20Driving,using%20only%20the%20accelerator%20pedal.)
  * Regenerative breaking is dynamically enabled/disabled based on inputs from BMS, MDI, and other switches on the MCB.
* Cruise control
  * Driver can enable cruise control by pressing a button on the steering wheel which sets the cruise speed to the current velocity of the car.
  * Cruise control can be increase/decreased by pressing buttons on the steering wheel.
  * Cruise control is automatically disabled on mechanical brake press.


Changes from `firmware_v2` to `firmware_v3`:
* FreeRTOS to improve time determinism and real-time performance
  * Tasks include handling the drive state machine, handling CAN I/O, and more.
* Interrupts used for most buttons and switch inputs rather than polling
* Bare-metal register write/reads have been replaced by HAL library API calls
* Abstraction and SWCs
  * Abstraction increases readability and maintainability of code.
  * Implemented complex system level code behind simplified interface.
  * Modular pieces of software that encapsulate specific functionality are separated into .c & .h files. No function is greater than 1.5 pages long.


<br>

## Driver Information Display (DID)

### DID v1.0
- Description of changes made to Component 2

<br>

## Array Monitor Board (AMB)

### AMB v1.0
- Description of changes made to Component 3

<br>

## Battery Management System (BMS)

### BMS v1.0

<br>

## Electronic Control Unit (ECU)

### ECU v1.0

<br>

## Telemetry Board (TEL)
