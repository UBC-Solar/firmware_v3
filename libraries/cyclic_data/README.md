# **Cyclic Data Library**

## Overview

This library provides a simple way to create, set, and retrieve cyclic data structures. It allows users to define cyclic data instances that store data values along with their cycle time, making it easy to check data validity based on elapsed time.


## Usage
### 1. Define a Cyclic Data Instance
```c
// Defines an float cyclic data with a 5-second maximum cycle time.
CYCLIC_DATA(float, cd_temperature, 5000);  
```
This creates a `float` cyclic data type named `cd_temperature` with a `max_cycle_time` of 5000 milliseconds.  
- Note: Cyclic data can be defined as any type. (`int`, `float`, `struct`)

### 2. Set Data Value
```c
// Sets the value to 25.1 and updates the timestamp.
CYCLIC_DATA_SET(cd_temperature, 25.1);
```
This updates `cd_temperature` to `25.1` and sets `last_set_time` to the current tick value returned by `HAL_GetTick()`.

### 3. Get Data Value
```c
// Get pointer to temperature
float* temperature = CYCLIC_DATA_GET(temperature_data);

// If temperature pointer is NULL, then the time elapsed since the last time data was updated exceeds the max_cycle_time.
if (temperature_ptr != NULL)
{
    printf("Temperature: %d\n", *temperature_ptr);
} else
{
    printf("ERROR: Timeout\n");
}
```
This checks if the data has been updated within the allowed cycle time. If not, it returns `NULL`.


## Notes
- This library uses `HAL_GetTick()`, so make sure your `SysTick` timer is configured properly.
- Be cautious with macro usage within complex expressions to avoid unintended behavior.