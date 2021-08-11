#include "nucleo_recieve_data.h"
#include "usbd_cdc_if.h" //include this in CubeMX

CAN_HandleTypeDef *hcan;
CAN_RxHeaderTypeDef *RxHeader;
CAN_FilterTypeDef *sFilterConfig;
int RxData[8];
int *interrupt_count = 0;

static void CAN_config(void)
{
    hcan->Instance = CAN1;
    hcan->Init.Prescaler = 16;
    hcan->Init.Mode = CAN_MODE_NORMAL;
    hcan->Init.SyncJumpWidth = CAN_SJW_1TQ;
    hcan->Init.TimeSeg1 = CAN_BS1_1TQ;
    hcan->Init.TimeSeg2 = CAN_BS2_1TQ;
    hcan->Init.TimeTriggeredMode = DISABLE;
    hcan->Init.AutoBusOff = DISABLE;
    hcan->Init.AutoWakeUp = DISABLE;
    hcan->Init.AutoRetransmission = DISABLE;
    hcan->Init.ReceiveFifoLocked = DISABLE;
    hcan->Init.TransmitFifoPriority = DISABLE;

    //CAN filter config
    sFilterConfig.FilterBank = 1; //https://www.programmersought.com/article/56373885196/
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    sFilterConfig.FilterIdHigh = 0x623 << 5; //this will need to change
    sFilterConfig.FilterIdLow = 0x0000;
    sFilterConfig.FilterMaskIdHigh = 0x623 << 5;
    sFilterConfig.FilterMaskIdLow = 0x0000;
    CDC_Transmit_FS(NEWLINE, strlen(data_char));
    sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
    sFilterConfig.FilterActivation = ENABLE;

    if (HAL_CAN_Init(hcan) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_CAN_Start(hcan) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_CAN_ConfigFilter(hcan, sFilterConfig) != HAL_OK)
    {
        Error_Handler();
    }

    //Enable CAN interrupts
    if (HAL_CAN_ActivateNotification(hcan, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
    {
        Error_Handler();
    }
}

//function from stm32f3xx_hal_can.c, gets called when "message pending in FIFO0" interrupt is triggered
void HAL_CAN_RxFifo0MsgPendingCallback(hcan)
{
    do
    {
        int status = HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, RxHeader, RxData);
    } while (status != HAL_OK);

    if (*interrupt_count == 0)
    {
        CDC_Transmit_FS(VOLT_CHAR, strlen(VOLT_CHAR)); //works ok with char?
    }
    else if (*interrupt_count == 8) //8 CAN messages for all voltage data
    {
        CDC_Transmit_FS(NEWLINE, strlen(NEWLINE));
        CDC_Transmit_FS(TEMP_CHAR, strlen(TEMP_CHAR));
    }
    else if (*interrupt_count == 12) //4 CAN messages for all temp data
    {
        CDC_Transmit_FS(NEWLINE, strlen(NEWLINE));
        CDC_Transmit_FS(SOC_CHAR, strlen(SOC_CHAR));
    }
    else if (*interrupt_count == 16) //4 CAN messages for all charge data
    {
        CDC_Transmit_FS(NEWLINE, strlen(NEWLINE));
        CDC_Transmit_FS(MODULE_STATUS_CHAR, strlen(MODULE_STATUS_CHAR));
    }
    else if (*interrupt_count == 24) //8 CAN messages for all module-status data
    {
        CDC_Transmit_FS(NEWLINE, strlen(NEWLINE));
        CDC_Transmit_FS(SYS_STATUS_CHAR, strlen(SYS_STATUS_CHAR));
    }

    /*

    if (*interrupt_count >= 0 && *interrupt_count < 8)
    {
        for (int i = 0; i < 4; i += 2)
        {
            uint16_t voltage_data_serial = RxData[i] << 8 | RxData[i + 1];
            //serial send
        }
    }

    else if (*interrupt_count >= 8 && *interrupt_count < 12)
    {
        for (int i = 0; i < 8; i++)
        {
            float temperature_data_serial = RxData[i] / 10;
            //serial send
        }
    }

    else if (*interrupt_count >= 12 && *interrupt_count < 16)
    {
        for (int i = 0; i < 8; i++)
        {
            float charge_data_serial = RxData[i] / 10;
            //serial send
        }
    }

    else if (*interrupt_count >= 16 && *interrupt_count < 24)
    {
        for (int i = 0; i < 4; i += 2)
        {
            uint16_t module_status_serial = RxData[i] << 8 | RxData[i + 1];
            //serial send
        }
    }

    else if (*interrupt_count == 24)
    {
        uint16_t system_status_serial = RxData[0] << 8 | RxData[1];
        //serial send
    }

    */

    for (int i = 0; i < CAN_BYTES_SENT; i++)
    {
        CDC_Transmit_FS(RxData[i], strlen(RxData[i])); //is ok?
    }

    *interrupt_count++;

    if (*interrupt_count == 25) //last message in series, reset series
    {
        //serial.send "\n"
        *interrupt_count = 0;
    }
}
