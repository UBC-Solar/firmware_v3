/*

CANbus_function.h
Author: Edward Ma (Github: RootBeer1313)

Purpose: Goes with CANbus_function.c
*/

#ifndef CANBUS_FUNCTION_H_
#define CANBUS_FUNCTION_H_


//If debugging in visual studio, uncomment the include below.
//Else, comment out the include.
//#include "CANbus_TESTING_ONLY.h"

#ifndef CANBUS_TESTING_ONLY_H_
#include <stdint.h>
#include "stm32f3xx_hal.h"
#include "ltc6813_btm.h"
//#include "stm32f3xx_hal_can.h"
#include <math.h>
#include <stdio.h>
#endif

#define TRUE  1
#define FALSE 0

//general technicalities
#define TWENTY_MILLISECONDS 20
#define TWO_HUNDRED_MILLISECONDS 200
#define ONE_THOUSAND_MILLISECONDS 1000

#define PH_MAX_VALUE 2147483647 //max value of int (the data type of tick)

#define CAN_INITIAL_ELITHION_SERIES_ADDRESS 600
#define CAN_UNUSED_EXT_ID 0x0DEADBEE //defined as "deadbee" to ensure new people understand that this value showing up is by design, and not a coincidence. Also, "deadbeef" doesn't fit the extended id bit size.
#define CAN_BRIGHTSIDE_DATA_LENGTH 8 //8 bytes

#define PH_SERIES_SIZE 2
#define CAN_ELITHION_MESSAGE_SERIES_SIZE 4 //Should be the number of unique messages

#define PH_TOTAL_MUX 32 //10 + 10 + 12 //to be replaced by another global constant that probably exists in another header file

#define CAN_STALE     1
#define CAN_NOT_STALE 0

#define CAN_REQUEST_ATTEMPT_MAX 3

//configuration in preprocessor instructions
#define CAN_ENABLE_REQUEST_QUEUE_REDUNDANCY 1



//message addresses

#define ADDRESS_622 622
#define ADDRESS_623 623
#define ADDRESS_626 626
#define ADDRESS_627 627

//value bounds
#define CAN_FAULT_VALUES            0b01111111

#define CAN_BITFLAG_FAULT_STATE     0b00000001 //bit 0

#define CAN_FAULTFLAG_OVERVOLTAGE   0b10000000 //bit 7
#define CAN_FAULTFLAG_UNDERVOLTAGE  0b01000000 //bit 6
#define CAN_FAULTFLAG_OVERTEMP      0b00100000 //bit 5
#define CAN_FAULTFLAG_COMMFAULT     0b00000100 //bit 2

#define CAN_WARNFLAG_HIGHTEMP       0b00100000 //bit 5
#define CAN_WARNFLAG_LOWTEMP        0b00010000 //bit 4
#define CAN_WARNFLAG_HIGHVOLTAGE    0b00000010 //bit 1
#define CAN_WARNFLAG_LOWVOLTAGE     0b00000001 //bit 0

#define CAN_PACK_MINIMUM 0 //0 kV
#define CAN_PACK_MAXIMUM 65000 //65 kV

#define CAN_MODULE_MINIMUM 0    //0 V
#define CAN_MODULE_MAXIMUM 255 // 25.5 V, units of 100mV

#define CAN_TEMPERATURE_MINIMUM -128 //twos complement; 0x1000'0000
#define CAN_TEMPERATURE_MAXIMUM 127 //twos complement; 0x0111'1111


/************************
Global variables
*************************/
CAN_HandleTypeDef * Brightside_CAN_handle; // &hcan //global pointer for easy inclusion.
BTM_PackData_t * CAN_PACKDATA_POINTER; // &pack

/************************
Structs
*************************/
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


/************************
external function prototypes
************************/
extern void CANstate_InitAll();
extern HAL_StatusTypeDef CANstate(Brightside_CAN_MessageSeries * pSeries);

/*
Copied from analysis.h, created by Andrew Hanlon.
Put here so that the code editor autocompletes properly.
*/

#ifdef SHAMELESSLY_COPIED_FROM_ANDREW_HANLON_ANALYSIS_H_
#define BMS_FAULT_COMM          0x0001
#define BMS_FAULT_ST            0x0002
#define BMS_FAULT_OT            0x0004
#define BMS_FAULT_UV            0x0008
#define BMS_FAULT_OV            0x0010
#define BMS_FAULT_NO_VOLT       0x0020
#define BMS_FAULT_TEMP_RANGE    0x0040

#define BMS_TRIP_BAL            0x0100 //(3) (flags) bit N/A
#define BMS_TRIP_LLIM           0x0200 //(3) (flags) bit 6
#define BMS_TRIP_HLIM           0x0400 //(3) (flags) bit 5

#define BMS_WARNING_LOW_V       0x1000
#define BMS_WARNING_HIGH_V      0x2000
#define BMS_WARNING_LOW_T       0x4000
#define BMS_WARNING_HIGH_T      0x8000
#endif
