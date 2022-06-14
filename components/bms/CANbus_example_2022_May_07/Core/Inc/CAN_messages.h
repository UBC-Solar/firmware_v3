#ifndef CAN_MESSAGES_H
#define CAN_MESSAGES_H

#include <stdint.h>
#include "headers_for_mocking.h"
#include "Pack_packdata.h"

#define MESSAGE624_SIZE 6


short CAN_getPackCurrent();

uint8_t * CAN_createMessage624();

uint8_t * CAN_createExpectedMessage624(short packCurrent);

#endif // CAN_MESSAGES_H
