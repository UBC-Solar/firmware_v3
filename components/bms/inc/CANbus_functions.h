#ifndef CANBUS_FUNCTION_H
#define CANBUS_FUNCTION_H

#include "stm32f3xx_hal.h"


#define TRUE = 1
#define FALSE = 0

//general technicalities
#define PH_START_OF_ADDRESS_SERIES 600
#define PH_UNUSED 0x0DEADBEE
#define CAN_BRIGHTSIDE_DATA_LENGTH 8

#define PH_SERIES_SIZE 2

#define PH_TOTAL_MUX = 32 //10 + 10 + 12

#define CAN_PENDING = 1
#define CAN_NOT_PENDING = 0
//message addresses
#define ADDRESS_623 = 623;
#define ADDRESS_627 = 627;

//value bounds
#define CAN_PACK_MINIMUM 0 //0 kV
#define CAN_PACK_MAXIMUM 65000 //65 kV

#define CAN_MODULE_MINIMUM 0    //0 V
#define CAN_MODULE_MAXIMUM 255 // 25.5 V, units of 100mV

#define CAN_TEMPERATURE_MINIMUM -128 //twos complement; 0x1000'0000
#define CAN_TEMPERATURE_MAXIMUM 127 //twos complement; 0x0111'1111


typedef struct
{
    CAN_TxHeaderTypeDef header;
    uint8_t dataFrame[CAN_BRIGHTSIDE_DATA_LENGTH];
    uint32_t mailbox;
}
Brightside_CAN_MessageTypeDef;


typedef structs
{
    //The struct holding the actual message data.
    Brightside_CAN_MessageTypeDef message[];
    //Convienient storage of the counter that tracks
    //how many of the messages have been requested.
    int runningIndex = 0;
    //the total messages in the stack.
    int messageStackSize;
}
Brightside_CAN_MessageSeries;
