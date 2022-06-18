#ifndef CAN_ONLYTESTINGFUNCTIONS_H
#define CAN_ONLYTESTINGFUNCTIONS_H

#include <stdint.h>
#include <stddef.h>
#include "headers_for_mocking.h"
#include "Pack_packdata.h"

#define MESSAGE624_SIZE 6
#define MESSAGE622_SIZE 7


short CAN_getPackCurrent();

uint8_t * CAN_createMessage624();

uint8_t * CAN_createExpectedMessage622withFaultFlag(uint8_t faultFlag);
uint8_t * CAN_createExpectedMessage622withBitFlag(uint8_t bitNumber);

uint8_t * CAN_createExpectedMessage624(short packCurrent);

#endif // CAN_MESSAGES_H
