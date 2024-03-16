#ifdef TEST

#include "mock_stm32f1xx_hal.h"
#include "mock_stm32f1xx_hal_conf.h"
#include "mock_stm32f1xx_hal_gpio.h"
#include "mock_stm32f1xx_hal_can.h"

#include "mock_adc.h"
#include "fsm.h"
#include "can.h"
#include <stdlib.h>

void setUp(void)
{
}

void tearDown(void)
{
}

void test_FSM_init(){
    HAL_GetTick_ExpectAndReturn(42);    
    FSM_init();
    TEST_ASSERT_EQUAL(FSM_RESET, FSM_state);
}


void test_FSM_reset(){

    FSM_state = FSM_RESET;

    HAL_GPIO_WritePin_Expect(FAN1_CTRL_GPIO_Port, FAN1_CTRL_Pin, LOW);
    HAL_GPIO_WritePin_Expect(FAN2_CTRL_GPIO_Port, FAN2_CTRL_Pin, LOW);
    HAL_GPIO_WritePin_Expect(FAN3_CTRL_GPIO_Port, FAN3_CTRL_Pin, LOW);
    HAL_GPIO_WritePin_Expect(FAN4_CTRL_GPIO_Port, FAN4_CTRL_Pin, LOW);
    HAL_GPIO_WritePin_Expect(MDUFAN_CTRL_GPIO_Port, MDUFAN_CTRL_Pin, LOW);

    HAL_GPIO_WritePin_Expect(HLIM_CTRL_GPIO_Port, HLIM_CTRL_Pin, LOW);
    HAL_GPIO_WritePin_Expect(LLIM_CTRL_GPIO_Port, LLIM_CTRL_Pin, LOW);
    HAL_GPIO_WritePin_Expect(DCDC_NEG_CTRL_GPIO_Port, DCDC_NEG_CTRL_Pin, LOW);
    HAL_GPIO_WritePin_Expect(DCDC_POS_CTRL_GPIO_Port, DCDC_POS_CTRL_Pin, LOW);
    HAL_GPIO_WritePin_Expect(NEG_CTRL_GPIO_Port, NEG_CTRL_Pin, LOW);
    HAL_GPIO_WritePin_Expect(PC_CTRL_GPIO_Port, PC_CTRL_Pin, LOW);

    
    uint32_t rand_tick = (uint32_t)rand();
    ADC_getSuppBattVoltage_ExpectAndReturn(9000);
    HAL_GPIO_ReadPin_ExpectAndReturn(SUPP_LOW_GPIO_Port, SUPP_LOW_Pin, LOW);
    ADC3_getFaultStatus_ExpectAndReturn(false);

    HAL_GPIO_WritePin_Expect(SUPP_LOW_GPIO_Port, SUPP_LOW_Pin, HIGH);
    HAL_GetTick_ExpectAndReturn(42);
    FSM_reset();
}






#endif // TEST