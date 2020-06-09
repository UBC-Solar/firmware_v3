#ifndef CANBUS_FUNCTION_H
#define CANBUS_FUNCTION_H

#include "stm32f3xx_hal.h"


#define CAN_PACK_MINIMUM 0 //0 kV
#define CAN_PACK_MAXIMUM 65 //65 kV

#define CAN_MODULE_MINIMUM 0    //0 V
#define CAN_MODULE_MAXIMUM 25.5 // 25.5 V

#define CAN_TEMPERATURE_MINIMUM -128 //twos complement; 0x1000'0000
#define CAN_TEMPERATURE_MAXIMUM 127 //twos complement; 0x0111'1111
