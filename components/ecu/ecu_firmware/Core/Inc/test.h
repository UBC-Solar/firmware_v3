#ifndef __TEST_H
#define __TEST_H

#include "stm32f1xx_hal.h"

void TEST_hvContactors();
void TEST_lvControl();
void TEST_fanControl();
void TEST_readCurrentReferences();
void TEST_estop();
void TEST_relays();
void TEST_can(CAN_HandleTypeDef *hcan);

#endif /* __TEST_H */