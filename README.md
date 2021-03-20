# firmware_v3

This repository contains all of the firmware for UBC Solar's third-generation vehicle, Brightside.

Each custom device on the car for which the team has written firmware has its own folder in the `/components/` directory. The following firmware projects can be found in this repository:

- BMS (Battery Management System)
    - bms_master - firmware for the master board of the team's first in-house BMS
    - SPITest - the precursor to bms_master, a prototype project where much of the bms firmware was initially developed using development boards

When adding a firmware project for another device on the car to this repository, start by creating a folder in the `/components/` directory named using the 3-letter identifier for the device (eg. `bms`, `ecu`, `mcb`), if one does not already exist. Then, place the firmware project for the device in the corresponding 3-letter ID folder.

In addition to the firmware for hardware on the car in `/components/`, any tools that have been developed for working with this hardware can be found in the `/tools/` folder. The following tools can be found here:

- BMS GUI - An interface program capable of displaying and logging information from the BMS read via a serial port, written in Python using PyQT.
## Contributing

The firmware projects in this repository (so far) are written in C and developed using either STM32CubeIDE or PlatformIO.
For information on getting set up to work on the projects that use STM32CubeIDE, please visit the team's [tutorial on STM32CubeIDE](https://sites.google.com/ubcsolar.com/project-management/tutorials/stm32cubeide).

So far, team members have utilized branches to facilitate work in parallel on this codebase, but as the repository grows we may switch to forks. Branches on the main repository should follow the naming scheme

`user/<name>/<project>/<feature>`

where the values delimited by `<>` should be replaced by your information. **No spaces please.** You may use your first name or your GitHub username for `<name>`. For example, `users/a2k-hanlon/bms/fsm`.

Once your contributions are error-free and ready to add to the master branch, submit a pull request so that another team member can review and merge your work.
