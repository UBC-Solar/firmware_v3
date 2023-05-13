# firmware_v3

This repository contains all of the firmware for UBC Solar's third-generation vehicle, Brightside.

Each custom device on the car for which the team has written firmware has its own folder in the `/components/` directory. The following firmware projects can be found in this repository:

- BMS (Battery Management System)
  - `bms_master` - firmware for the master board of the team's first in-house BMS

- ECU (Electronic Control Unit) for the battery "control board" - the section of the battery with all of the high-voltage switching circuitry
  - `ecu-firmware` - firmware for the microcontroller on the ECU that manages all of the switching components in the battery

When adding a firmware project for another device on the car to this repository, start by creating a folder in the `/components/` directory named using the 3-letter identifier for the device (eg. `bms`, `ecu`, `mcb`), if one does not already exist. Then, place the firmware project for the device in the corresponding 3-letter ID folder.

In addition to the firmware for hardware on the car in `/components/`, any tools that have been developed for working with this hardware can be found in the `/tools/` folder. The following tools can be found in this repository:

- BMS GUI - An interface program capable of displaying and logging information from the BMS read via a serial port, written in Python using PyQT.
- Ceedling - The team's copy of the open source unit testing framework. See below for more info on Ceedling.

## Contributing

The firmware projects in this repository (so far) are written in C and developed using either STM32CubeIDE or PlatformIO.
For information on getting set up to work on the projects that use STM32CubeIDE, please visit the team's [tutorial on STM32CubeIDE](https://wiki.ubcsolar.com/tutorials/stm32cubeide).

Some pieces of firmware also have unit tests implemented with [Ceedling](https://github.com/ThrowTheSwitch/Ceedling). For information on unit testing using Ceedling, please visit the team's [tutorial on Ceedling](https://wiki.ubcsolar.com/tutorials/ceedling-unit-testing).

Team members create branches directly on this repository (rather than repository forks) to facilitate work in parallel on this codebase. Branches on the repository should follow the naming scheme

`user/<name>/<project>/<feature>`

where the values delimited by `<>` should be replaced by your information. **No spaces please.** You may use your first name or your GitHub username for `<name>`. For example, `user/a2k-hanlon/bms/fsm`.

Once your contributions are error-free and ready to add to the master branch, submit a pull request so that another team member can review and approve your work, allowing you you merge it.
