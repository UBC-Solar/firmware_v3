# CPU Load Library

## Overview

The CPU Load Library is designed to calculate the CPU load of the connected MCU. This library measures the load at a user-specified frequency (in milliseconds) and utilizes a circular buffer for a windowed approach to track CPU loads over time. The buffer's values are averaged to provide a real-time CPU load measurement.

## Prerequisite

Make sure to link the library folder. A tutorial on how to do that can be found [here.](https://wiki.ubcsolar.com/en/tutorials/stm32cubeide)

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

1. **Call Initialization Function**:
   - In `freeRTOS.c`, locate the `MX_FREERTOS_Init` function and call `CPU_LOAD_init()`, passing in:
     - A window size (integer from 0 to 20).
     - A frequency (in milliseconds) for CPU load calculation.
     - A pointer to the timer handler, e.g., `&htim2` if using Timer 2.

### Step 4 (Optional): Getting the average CPU load from the buffer

   If you are interested in getting the average of the CPU loads in the circular buffer, you can simply call the averaging function and store it in a variable.
   For example:
   
   ```c
     float average_cpu_load = CPU_LOAD_average();
   ```

The library is now set up to calculate and track the CPU load of the MCU at your specified frequency!
