# CPU Load Library

## Overview

The CPU Load Library is designed to calculate the CPU load of the connected MCU. This library measures the load at a user-specified frequency (in milliseconds) and utilizes a circular buffer for a windowed approach to track CPU loads over time. The buffer's values are averaged to provide a real-time CPU load measurement.

## Prerequisite

Make sure to link the library folder. A tutorial on how to do that can be found [here.](https://wiki.ubcsolar.com/en/tutorials/stm32cubeide)

## Known Issues
As of December 4th, 2024, here are some known issues with the library. 
* After every 20-30 CPU load values are sent out, a negative (or very small) CPU load average may be sent out over CAN. This is because every so often, the tick count of the timer used for tracking the CPU load exceeds the number of ticks in the window we are checking CPU load in. As a result, a negative value is added to the CPU Load circular buffer and thus the average may come out negative when you call `CPU_LOAD_average`. **We currently do not know why this occurs**. 

## Usage

### Step 1: Include Required Headers

1. **Include Header Files**: 
   - Add `cpu_load.h` to your `main.c` and `freeRTOS.c` files.
   - Add `cpu_load_trace.h` to the bottom-most user code section of `FreeRTOSConfig.h`.

2. **Enable FreeRTOS Macros**:
   - Copy and paste the following definition at the bottom of the API function section on line 97 in `FreeRTOSConfig.h`:
     ```c
     #define INCLUDE_xTaskGetIdleTaskHandle 1
     ```

### Step 2: Set Up a Timer

1. **Configure Timer**:
   - Choose a timer and configure it to tick every 1 **microsecond**. For example, with a 72 MHz timer, set the prescaler to 71.

2. **Handle Timer Overflow**:
   - In the `HAL_TIM_PeriodElapsedCallback` function, call:
     ```c
     CPU_LOAD_timer_overflow_handler(htim);
     ```

### Step 3: Initialize the Component

1. **Initialize CPU Load Config Struct
   - In freeRTOS.c, locate the MX_FREERTOS_Init functio and 
     create your "user configuration" by initializing a new 
     struct of type CPU_LOAD_config_t and set the window size, frequency, and timer, to your desires.
     For example:

   ```c
   CPU_LOAD_config_t user_config = {
      .window_size = 10,
      .frequency_ms = 100,
      .timer = htim2
   };
   ```

2. **Call Initialization Function**:
   - In `freeRTOS.c`, locate the `MX_FREERTOS_Init` function and call `CPU_LOAD_init()`, passing in a pointer to the user config that you just created.
   For example:

   ```c
   CPU_LOAD_init(&user_config);
   ```
   
### Step 4 (Optional): Getting the average CPU load from the buffer

   If you are interested in getting the average of the CPU loads in the circular buffer, you can simply call the averaging function and store it in a variable.
   For example:
   
   ```c
     float average_cpu_load = CPU_LOAD_average();
   ```

The library is now set up to calculate and track the CPU load of the MCU at your specified frequency!
