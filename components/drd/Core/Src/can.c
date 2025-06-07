/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.c
  * @brief   This file provides code for the configuration
  *          of the CAN instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "can.h"

/* USER CODE BEGIN 0 */
#include "CAN_comms.h"
#include "external_lights.h"
#include "fault_lights.h"
#include "drive_state.h"
#include "diagnostic.h"
#include "lcd.h"
#include "soc.h"
#include "cyclic_data_handler.h"

//GLOBAL VARIABLES
volatile float g_pack_current_A  = 0.0f;   // signed,  A 
volatile float g_array_current_A = 0.0f;   // summed, A 

/**
 * 	CAN message header for a drive command. This command header is to
 * 	send an appropriate drive command to the motor controller.
 */
const CAN_TxHeaderTypeDef drive_command_header = {
    .StdId = MOTOR_DRIVE_COMMAND_ADDRESS,
    .ExtId = 0x0000,
    .IDE = CAN_ID_STD,
    .RTR = CAN_RTR_DATA,
    .DLC = DRIVE_COMMAND_SIZE

};

/*
 * CAN message header for an MDU request. This message is sent to the Mitsuba motor controller to query
 * 		needed data from it.
 */
const CAN_TxHeaderTypeDef mdu_request_header = {
		.StdId = 0,
		.ExtId = MDU_REQUEST_COMMAND_ID,
		.IDE = CAN_ID_EXT,
		.RTR = CAN_RTR_DATA,
		.DLC = MDU_REQUEST_SIZE
};


const CAN_TxHeaderTypeDef drd_diagnostic_header = {
		.StdId = DRD_DIAGNOSTIC_MESSAGE,
		.ExtId = 0x0000,
		.IDE = CAN_ID_STD,
		.RTR = CAN_RTR_DATA,
		.DLC = DRD_DIAGNOSTIC_SIZE

};


const CAN_TxHeaderTypeDef time_since_bootup_can_header = {
   .StdId = TIME_SINCE_BOOTUP_CAN_ID,
   .ExtId = 0x0000,
   .IDE = CAN_ID_STD,
   .RTR = CAN_RTR_DATA,
   .DLC = TIME_SINCE_BOOTUP_CAN_DATA_LENGTH};

/* USER CODE END 0 */

CAN_HandleTypeDef hcan;

/* CAN init function */
void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN1;
  hcan.Init.Prescaler = 8;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_4TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_4TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = ENABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = ENABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN_Init 2 */

  /* USER CODE END CAN_Init 2 */

}

void HAL_CAN_MspInit(CAN_HandleTypeDef* canHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspInit 0 */

  /* USER CODE END CAN1_MspInit 0 */
    /* CAN1 clock enable */
    __HAL_RCC_CAN1_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**CAN GPIO Configuration
    PB8     ------> CAN_RX
    PB9     ------> CAN_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    __HAL_AFIO_REMAP_CAN1_2();

    /* CAN1 interrupt Init */
    HAL_NVIC_SetPriority(USB_HP_CAN1_TX_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USB_HP_CAN1_TX_IRQn);
    HAL_NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
  /* USER CODE BEGIN CAN1_MspInit 1 */

  /* USER CODE END CAN1_MspInit 1 */
  }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* canHandle)
{

  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspDeInit 0 */

  /* USER CODE END CAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CAN1_CLK_DISABLE();

    /**CAN GPIO Configuration
    PB8     ------> CAN_RX
    PB9     ------> CAN_TX
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8|GPIO_PIN_9);

    /* CAN1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USB_HP_CAN1_TX_IRQn);
    HAL_NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn);
  /* USER CODE BEGIN CAN1_MspDeInit 1 */

  /* USER CODE END CAN1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */


/**
  * @brief  Initializes CAN filters to allow only the desired IDs.
  *         Standard IDs allowed:
  *         0x401, 0x450, 0x622, 0x623, 0x624, 0x580
  *         Extended IDs allowed:
  *           0x08A50225, 0x08850225
  *
  * For standard IDs we use 16-bit scale in ID list mode (each ID must be shifted left by 5).
  * For extended IDs we use 32-bit scale in ID list mode; the 29-bit ID is shifted left by 3 and the IDE bit is set.
  */
 void CAN_filter_init(CAN_FilterTypeDef* can_filter)
 {
   CAN_FilterTypeDef can_filter1;
   CAN_FilterTypeDef can_filter2;
 
 
   // ---- Filter Bank 0 ----
   can_filter->FilterIdHigh = (CAN_ID_BATT_FAULTS << 5);
   can_filter->FilterMaskIdHigh = (CAN_ID_PACK_VOLTAGE << 5);
   can_filter->FilterIdLow = (CAN_ID_PACK_HEALTH << 5);
   can_filter->FilterMaskIdLow = (CAN_ID_PACK_CURRENT << 5);
   can_filter->FilterFIFOAssignment = CAN_FILTER_FIFO0;
   can_filter->FilterBank = 0;
   can_filter->FilterMode = CAN_FILTERMODE_IDLIST;
   can_filter->FilterScale = CAN_FILTERSCALE_16BIT;
   can_filter->FilterActivation = ENABLE;
   HAL_CAN_ConfigFilter(&hcan, can_filter);
 
   // ---- Filter Bank 4 ----
   can_filter1.FilterIdHigh = (STR_CAN_MSG_ID << 5);
   can_filter1.FilterMaskIdHigh = (STR_CAN_MSG_ID << 5);
   can_filter1.FilterIdLow = (STR_CAN_MSG_ID << 5);
   can_filter1.FilterMaskIdLow = (STR_CAN_MSG_ID << 5);
   can_filter1.FilterFIFOAssignment = CAN_FILTER_FIFO0;
   can_filter1.FilterBank = 4;
   can_filter1.FilterMode = CAN_FILTERMODE_IDLIST;
   can_filter1.FilterScale = CAN_FILTERSCALE_16BIT;
   can_filter1.FilterActivation = ENABLE;
   HAL_CAN_ConfigFilter(&hcan, &can_filter1);
 
   // ---- Filter Bank 2 ----
   uint32_t extId1 = CAN_ID_MTR_FAULTS;
   uint32_t extId2 = FRAME0;
   can_filter2.FilterIdHigh = (extId1 << 3) >> 16;
   can_filter2.FilterIdLow  = ((extId1 << 3) & 0xFFFF) | 0x0004;
   can_filter2.FilterMaskIdHigh = (extId2 << 3) >> 16;
   can_filter2.FilterMaskIdLow  = ((extId2 << 3) & 0xFFFF) | 0x0004;
   can_filter2.FilterFIFOAssignment = CAN_FILTER_FIFO0;
   can_filter2.FilterBank = 5;
   can_filter2.FilterMode = CAN_FILTERMODE_IDLIST;
   can_filter2.FilterScale = CAN_FILTERSCALE_32BIT;
   can_filter2.FilterActivation = ENABLE;
   HAL_CAN_ConfigFilter(&hcan, &can_filter2);
 }


/**
 * @brief Initializes the CAN filter and CAN Rx callback function as CAN_comms_Rx_callback().
 *
 * Note: This uses the CAN_comms abstraction layer which will initialize two freeRTOS tasks. As a result it is recommended to
 * Call this function inside the MX_FREERTOS_Init() function in freertos.c
 */
void CAN_tasks_init()
{
    CAN_comms_config_t CAN_comms_config_tel = {0};
    CAN_FilterTypeDef can_filter = {0};
    CAN_filter_init(&can_filter);

    CAN_comms_config_tel.hcan = &hcan;
    CAN_comms_config_tel.CAN_Filter = can_filter;
    CAN_comms_config_tel.CAN_comms_Rx_callback = CAN_comms_Rx_callback;

    CAN_comms_init(&CAN_comms_config_tel);
}


void CAN_comms_Rx_callback(CAN_comms_Rx_msg_t* CAN_comms_Rx_msg)
{
	uint32_t CAN_ID = 0;
	/*
	 *	handle parsing rx messages
	 */
	if (CAN_comms_Rx_msg == NULL)
	{
			return;
	}

	if(CAN_comms_Rx_msg->header.IDE == CAN_ID_EXT)
	{
		CAN_ID = CAN_comms_Rx_msg->header.ExtId; // Get CAN ID
	}
	else
	{
		CAN_ID = CAN_comms_Rx_msg->header.StdId; // Get CAN ID
	}

    /* PACK CURRENT */
    if (CAN_ID == CAN_ID_PACK_CURRENT)
    {
        int16_t raw =  (CAN_comms_Rx_msg->data[1] << 8) |
                    (CAN_comms_Rx_msg->data[0]);

        /* 1.  Smart-regen consumes the pack current for its analysis (A) */
        g_pack_current_A = (float)raw / 65.535f;

        /* 2.  Everybody else still uses int16_t amps  */
        int16_t pack_A = (int16_t)(g_pack_current_A + 0.5f);
        set_cyclic_pack_current(pack_A);
        g_pack_current_soc = pack_A;
    }

    /* MPPT ARRAY CURRENTS */ 
    if ((CAN_ID == MPPT_A_CURRENTS) || (CAN_ID == MPPT_B_CURRENTS) || (CAN_ID == MPPT_C_CURRENTS))
    {
        /* bytes 4-7 hold a little-endian IEEE-754 float */
        union { uint8_t b[4]; float f; } u;
        u.b[0] = CAN_comms_Rx_msg->data[4];
        u.b[1] = CAN_comms_Rx_msg->data[5];
        u.b[2] = CAN_comms_Rx_msg->data[6];
        u.b[3] = CAN_comms_Rx_msg->data[7];

        static float mppt_currents_array[3] = {0};          /* A,B,C  */
        uint8_t idx = (CAN_ID == MPPT_A_CURRENTS) ? 0 :
                        (CAN_ID == MPPT_B_CURRENTS) ? 1 : 2;
        mppt_currents_array[idx] = u.f;

        g_array_current_A = mppt_currents_array[0] + mppt_currents_array[1] + mppt_currents_array[2];
    }


	LCD_CAN_rx_handle(CAN_ID, CAN_comms_Rx_msg->data);
	Fault_Lights_CAN_rx_handle(CAN_ID, CAN_comms_Rx_msg->data);
	External_Lights_CAN_rx_handle(CAN_ID, CAN_comms_Rx_msg->data);
	Vehicle_State_CAN_rx_handle(CAN_ID, CAN_comms_Rx_msg->data);
}

/* USER CODE END 1 */
