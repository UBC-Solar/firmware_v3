#ifdef TEST

#include "mock_stm32f1xx_hal.h"
#include "mock_stm32f1xx_hal_conf.h"
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_can.h"

#include "fsm.h"
#include "adc.h"
#include "can.h"

void setUp(void)
{
    //GPIO_InitTypeDef GPIO_InitStruct = {0};
}

void tearDown(void)
{
}


#endif // TEST
