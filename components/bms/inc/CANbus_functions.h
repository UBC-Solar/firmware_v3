#ifndef CANBUS_FUNCTION_H
#define CANBUS_FUNCTION_H

#include "stm32f3xx_hal.h"



//general technicalities
#define PH_START_OF_ADDRESS_SERIES 600
#define PH_UNUSED 0x0DEADBEE
#define CAN_BRIGHTSIDE_DATA_LENGTH 8

#define PH_SERIES_SIZE 2

//message addresses
#define ADDRESS_623 = 623;
#define ADDRESS_627 = 627;

//value bounds
#define CAN_PACK_MINIMUM 0 //0 kV
#define CAN_PACK_MAXIMUM 65 //65 kV

#define CAN_MODULE_MINIMUM 0    //0 V
#define CAN_MODULE_MAXIMUM 25.5 // 25.5 V

#define CAN_TEMPERATURE_MINIMUM -128 //twos complement; 0x1000'0000
#define CAN_TEMPERATURE_MAXIMUM 127 //twos complement; 0x0111'1111


typedef struct
{
    CAN_TxHeaderTypeDef messageHeader;
    uint8_t dataFrame[CAN_BRIGHTSIDE_DATA_LENGTH];
}
Brightside_CAN_MessageTypeDef;
