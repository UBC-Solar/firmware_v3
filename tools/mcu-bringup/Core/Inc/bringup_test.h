#ifndef __BRINGUP_TEST_H
#define __BRINGUP_TEST_H

#include "stm32f1xx.h"

void BringupTest_blinkLed(void);
void BringupTest_gpioSequence(void);
void BringupTest_fanPwm(TIM_HandleTypeDef *htim);
void BringupTest_spiTx(SPI_HandleTypeDef *hspi);
void BringupTest_canTx(CAN_HandleTypeDef *hcan);
void BringupTest_uartTx(void);

#endif // __BRINGUP_TEST_H
