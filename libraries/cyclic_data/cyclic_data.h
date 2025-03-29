/**
 * @file cyclic_data.h
 * 
 * @brief cyclic data library
 * This library contains a header that a user can include to create cyclic data types.
 * 
 * @attention See README for details on how to implement this library.
 */

 #ifndef __CYCLIC_DATA_H__
 #define __CYCLIC_DATA_H__
 
 
 /* Includes */
 #include <stdint.h>
 #include "stm32f1xx_hal.h"
 
 
 /**
  * @brief Defines a cyclic data structure for storing a value along with its cycle time.
  * 
  * This macro creates a struct with the specified datatype that stores:
  * - The data itself
  * - The cycle time in milliseconds
  * - The last time the data was set
  * 
  * @param datatype The type of the data to be stored (e.g., int, float, etc.).
  * @param name The name of the cyclic data instance (e.g., temperature_data, speed_data, etc.).
  * @param max_cycle_time The maximum cycle time in milliseconds.
  */
 #define CYCLIC_DATA(datatype, name, max_cycle_time)               \
     struct {                                                      \
         datatype data;                                            \
         uint32_t cycle_time;                                      \
         uint32_t last_set_time;                                   \
     } name = { .cycle_time = max_cycle_time, .last_set_time = 0 }
 
 
 /**
  * @brief Sets cyclic data value and updates the timestamp.
  * 
  * This macro updates the data field of the specified cyclic data structure and records the 
  * time at which the data was set by calling `HAL_GetTick()`.
  * 
  * @param name The name of the cyclic data instance (created using CYCLIC_DATA).
  * @param new_data The new value to be stored in the cyclic data instance.
  */
 #define CYCLIC_DATA_SET(name, new_data)     \
     do {                                    \
         name.data = new_data;               \
         name.last_set_time = HAL_GetTick(); \
     } while(0)
 
 
 /**
  * @brief Retrieves a pointer to the cyclic data if it's within the valid cycle time.
  * 
  * This macro checks if the current time (retrieved using `HAL_GetTick()`) is within 
  * the acceptable range of the last set time plus the maximum cycle time. 
  * 
  * If the current time exceeds the allowed cycle time, it returns `NULL`. 
  * Otherwise, it returns a pointer to the data.
  * 
  * @param name The name of the cyclic data instance (created using CYCLIC_DATA).
  * 
  * @return Pointer to the data if it's within the valid cycle time, otherwise NULL.
  */
 #define CYCLIC_DATA_GET(name) (HAL_GetTick() > (name.cycle_time + name.last_set_time) ? &(name.data) : NULL)
 
 
 #endif
 