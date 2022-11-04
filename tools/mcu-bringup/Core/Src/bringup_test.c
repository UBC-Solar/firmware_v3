#include "bringup_test.h"
#include "main.h"

#include <stdio.h>

#define FAN_PWM_TIMER_CHANNEL TIM_CHANNEL_3

void BringupTest_blinkLed(void)
{
    HAL_GPIO_TogglePin(LED_OUT_GPIO_Port, LED_OUT_Pin);
    HAL_Delay(500);
}

void BringupTest_gpioSequence(void)
{
    HAL_GPIO_WritePin(GPIO_OUT_GPIO_Port, GPIO_OUT_Pin, 1);
    HAL_Delay(100);

    HAL_GPIO_WritePin(GPIO_OUT_GPIO_Port, GPIO_OUT_Pin, 0);
    HAL_GPIO_WritePin(OT_OUT_GPIO_Port, OT_OUT_Pin, 1);
    HAL_Delay(100);

    HAL_GPIO_WritePin(OT_OUT_GPIO_Port, OT_OUT_Pin, 0);
    HAL_GPIO_WritePin(BAL_OUT_GPIO_Port, BAL_OUT_Pin, 1);
    HAL_Delay(100);

    HAL_GPIO_WritePin(BAL_OUT_GPIO_Port, BAL_OUT_Pin, 0);
    HAL_GPIO_WritePin(FLT_OUT_GPIO_Port, FLT_OUT_Pin, 1);
    HAL_Delay(100);

    HAL_GPIO_WritePin(FLT_OUT_GPIO_Port, FLT_OUT_Pin, 0);
    HAL_GPIO_WritePin(LLIM_OUT_GPIO_Port, LLIM_OUT_Pin, 1);
    HAL_Delay(100);

    HAL_GPIO_WritePin(LLIM_OUT_GPIO_Port, LLIM_OUT_Pin, 0);
    HAL_GPIO_WritePin(HLIM_OUT_GPIO_Port, HLIM_OUT_Pin, 1);
    HAL_Delay(100);

    HAL_GPIO_WritePin(HLIM_OUT_GPIO_Port, HLIM_OUT_Pin, 0);
    HAL_GPIO_WritePin(COM_OUT_GPIO_Port, COM_OUT_Pin, 1);
    HAL_Delay(100);

    HAL_GPIO_WritePin(COM_OUT_GPIO_Port, COM_OUT_Pin, 0);
}

void BringupTest_fanPwm(TIM_HandleTypeDef *htim)
{
    __HAL_TIM_SET_COMPARE(htim, FAN_PWM_TIMER_CHANNEL, 25);
    HAL_Delay(1000);
    __HAL_TIM_SET_COMPARE(htim, FAN_PWM_TIMER_CHANNEL, 50);
    HAL_Delay(1000);
    __HAL_TIM_SET_COMPARE(htim, FAN_PWM_TIMER_CHANNEL, 75);
    HAL_Delay(1000);
    __HAL_TIM_SET_COMPARE(htim, FAN_PWM_TIMER_CHANNEL, 100);
    HAL_Delay(1000);
    __HAL_TIM_SET_COMPARE(htim, FAN_PWM_TIMER_CHANNEL, 0);
    HAL_Delay(1000);
}

// SPI
void BringupTest_spiTx(SPI_HandleTypeDef *hspi)
{
    HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, 0);
    uint8_t data[] = {0x55, 0xAA};
    HAL_SPI_Transmit(hspi, data, 2, 0xFFFFFFFF);
    HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, 1);
    HAL_Delay(200);
}

// CAN
void BringupTest_canTx(CAN_HandleTypeDef *hcan)
{
    CAN_TxHeaderTypeDef header = {
        .StdId = 0x628,
        .ExtId = 0x0,
        .IDE = CAN_ID_STD,
        .RTR = CAN_RTR_DATA,
        .DLC = 4,
        .TransmitGlobalTime = DISABLE
    };
    uint8_t data[] = {0x55, 0xAA, 0xBE, 0xEF};
    uint32_t mailbox;
    HAL_CAN_AddTxMessage(hcan, &header, data, &mailbox);
    HAL_Delay(1000);
}

// UART
// Initialize DebugIO with the 
void BringupTest_uartTx(void)
{
    int count = 0;
    printf("Test %d\r\n", count);
    count++;
    HAL_Delay(1000);
}
