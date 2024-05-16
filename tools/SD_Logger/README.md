# SD Logger

## Purpose
The SD Logger is designed to capture CAN messages from the CAN bus, convert them to strings, and write them to an SD Card for logging purposes. This project aims to provide quick-access data logs for immediate visualization, with integration into the Solar Car to start and stop logging based on the car's power state.

## Requirements

### Hardware Requirements
- SD Card
- STM32 Nucleo board (Master) with SPI peripheral (Slave) for SD Card
- PCAN adapter
- Male-female jumper wires for CS, MOSI, MISO, GND, VCC connections

### Software Requirements
- Send/Create messages on a CAN bus using PCAN View
- Write firmware to retrieve CAN messages from a pre-existing queue (using STM32Cube IDE)
- Convert CAN messages to strings and write them to the SD Card
- Potential use of FreeRTOS for real-time processing
  
### Writing to the SD Card with SPI
The Serial Peripheral Interface (SPI) is a synchronous serial communication protocol 
used for interacting with peripherals like the SD cards. Below is a general procedure for writing data to an SD card using SPI:

1. **Initialize the SPI Peripheral:**
   - Configure the pins on the Nucleo with the `.ioc` file on STM32Cube IDE to have two UART pins.
   - Ensure FATFS is included in the project on the IDE
   - Match CS (Chip Select pin), MISO, and MOSI pins to the correct pins on the Nucleo based on the pin layout.
     
2. **Send the Data:**
   - Use PCAN view to create and send a CAN message to the Nucleo.
   - Write the data to the SD Card.
   - Ensure that the data is formatted according to the SD card's data format.

3. **End the SPI Communication:**
   - When the car is powering off, disable the SPI peripheral by setting CS to high.

## Assumptions
- Log file is created at the start of the car (when the program runs) and closed at the car's stop.
- Multiple start-stop sequences result in an equal number of log files.
- Random numbers for file naming prevent collisions.
- CAN messages are sent by a Nucleo board.

## Proposed Concept

### Concept 1 (NOT WORKING PSEUDO-CODE)
```c
void logCANMessages() {
    char CAN_as_string[SIZE_OF_MESSAGE];
    CAN_message_t current_can_message;

    while (true) {
        // Wait for a message to be available in the queue.
        bool hasMessage = doesQueueHaveMessage();

        if (hasMessage) {
            // The message was successfully read from the queue.
            // Convert the CAN message data to a string.
            CAN_as_string = queueMessageToString(current_can_message);

            // Open the file for writing.
            FILE* file = openFile();

            // Write the string to the file.
            writeToFile(file, CAN_as_string);

            // Close the file.
            fclose(file);
        }
    }
}
