#ifndef CAN_ONLYTESTINGFUNCTIONS_H
#define CAN_ONLYTESTINGFUNCTIONS_H

#include <stdint.h>
#include <stddef.h>
#include "headers_for_mocking.h"
#include "Pack_packdata.h"

// #define MESSAGE622_SIZE 7
// #define MESSAGE623_SIZE 6
// // #define MESSAGE623_SIZE 6
// #define MESSAGE624_SIZE 6
// #define MESSAGE626_SIZE 7
// #define MESSAGE627_SIZE 6

#define MESSAGE622_SIZE 8
#define MESSAGE623_SIZE 8
#define MESSAGE624_SIZE 8
#define MESSAGE626_SIZE 8
#define MESSAGE627_SIZE 8



extern uint8_t expectedMessage624[MESSAGE624_SIZE];
extern uint8_t message624[MESSAGE624_SIZE];

extern uint8_t expectedMessage622[MESSAGE622_SIZE];
extern uint8_t expectedMessage623[MESSAGE623_SIZE];
extern uint8_t PH_message623[MESSAGE623_SIZE];
extern uint8_t expectedMessage626[MESSAGE626_SIZE];
extern uint8_t expectedMessage627[MESSAGE627_SIZE];

short CAN_getPackCurrent();

uint8_t * CAN_createMessage624();

uint8_t * CAN_createExpectedMessage622withFaultFlag(uint8_t faultFlag);
uint8_t * CAN_createExpectedMessage622withBitFlag(uint8_t bitNumber);
uint8_t * CAN_createExpectedMessage622withTime(uint16_t time);

uint8_t * CAN_createIdealMessage622withFaultFlag(uint8_t faultFlag);
uint8_t * CAN_createIdealMessage622withBitFlag(uint8_t bitNumber);
uint8_t * CAN_createIdealMessage622withTime(uint16_t time);

uint8_t * CAN_createExpectedMessage623
(
    uint16_t totalPackVoltage,
    uint8_t lowestChargedVoltage,
    uint8_t IDofLowestVoltageCell,
    uint8_t highestChargedVoltage,
    uint8_t IDofHighestVoltageCell
);

uint8_t * CAN_createExpectedMessage624(short packCurrent);

uint8_t * CAN_createExpectedMessage626
(
    uint8_t StateOfCharge,
    uint16_t DepthOfDischarge,
    uint16_t Capacity
);

uint8_t * CAN_createExpectedMessage627
(
    uint8_t temperature,
    uint8_t minTemp,
    uint8_t IDofMinTempCell,
    uint8_t maxTemp,
    uint8_t IDofMaxTempCell
);

voltageInfoStruct * NONMOCKVERSION_setVoltageInfo
(
    voltageInfoStruct * voltageInfo,
    uint16_t MinVoltage,
    uint16_t MaxVoltage,
    uint8_t MinStackIndex,
    uint8_t MinModuleIndex,
    uint8_t MaxStackIndex,
    uint8_t MaxModuleIndex
);

#endif // CAN_MESSAGES_H
