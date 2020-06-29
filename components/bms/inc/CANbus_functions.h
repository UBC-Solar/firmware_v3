#ifndef CANBUS_FUNCTION_H_
#define CANBUS_FUNCTION_H_


//If debugging in visual studio, uncomment the include below.
//Else, comment out the include.
#include "CANbus_TESTING_ONLY.h"

#ifndef CANBUS_TESTING_ONLY_H_
#include <stdint.h>
#include "stm32f3xx_hal.h"
#include "stm32f3xx_hal_can.h"
#include <math.h>
#include <stdio.h>
#endif

#define TRUE = 1
#define FALSE = 0

//general technicalities
#define PH_START_OF_ADDRESS_SERIES 600
#define PH_UNUSED 0x0DEADBEE
#define CAN_BRIGHTSIDE_DATA_LENGTH 8

#define PH_SERIES_SIZE 2

#define PH_TOTAL_MUX 32 //10 + 10 + 12

#define CAN_PENDING     1
#define CAN_NOT_PENDING 0
//message addresses
#define ADDRESS_623 623;
#define ADDRESS_627 627;

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
    uint8_t * dataFrame;
    uint32_t mailbox;
}
Brightside_CAN_Message;


typedef struct
{
    //Pointer to the struct holding the actual message data.
    Brightside_CAN_Message * message;

    //Used to track which messages have been placed in
    //the TxMailboxes across multiple function calls.
    int runningIndex;

    //the total messages in the stack.
    int messageSeriesSize;
}
Brightside_CAN_MessageSeries;


#endif
