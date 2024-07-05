/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.c
  * @brief   This file provides code for the configuration
  *          of the CAN instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "common.h"
#include "main.h"
#include "string.h"
#include <stdlib.h>

// CAN message headers for VDS

CAN_TxHeaderTypeDef shock_travel_header = {
		.StdId = CAN_ID_SHOCK_TRAVEL,
		.ExtId = 0x0000,
	    .IDE = CAN_ID_STD,
	    .RTR = CAN_RTR_DATA,
	    .DLC = CAN_DATA_LENGTH};

CAN_TxHeaderTypeDef brake_and_steering_header = {
		.StdId = CAN_ID_BRAKE_AND_STEERING,
		.ExtId = 0x0000,
		.IDE = CAN_ID_STD,
		.RTR = CAN_RTR_DATA,
		.DLC = CAN_DATA_LENGTH};

CAN_TxHeaderTypeDef vds_diagnostic_header = {
		.StdId = CAN_ID_VDS_DIAGNOSTIC,
		.ExtId = 0x0000,
		.IDE = CAN_ID_STD,
		.RTR = CAN_RTR_DATA,
		.DLC = 1};

uint32_t can_mailbox;
VDS_Data_t can_data = {0};
VDS_Data_t can_data_2 = {0};
VDS_Data_t can_incoming_data = {0};

void CAN_SendShockTravel(VDS_Data_t *incoming_data)
{
	memcpy((void *)&can_data_2, (void *)incoming_data, sizeof(VDS_Data_t));


    CAN_Message_t message ={0};
    message.header = shock_travel_header;

    ADC_Value_t adc_value ={0};
    uint16_t *shock_travel_ptr = (uint16_t*) &can_data_2.adc_data.ADC_shock_travel_1;

    for (int i = 0; i < 4; ++i)
    {
        adc_value.value = *(shock_travel_ptr + i);
        message.data[i * 2] = adc_value.bytes.low;
        message.data[i * 2 + 1] = adc_value.bytes.high;
    }

    HAL_CAN_AddTxMessage(&hcan2, &message.header, message.data, &can_mailbox);
}

void CAN_SendBrakeAndSteering(VDS_Data_t *incoming_data)
{
	uint16_uint8_t ADC_brake_pressure_union_1 = {0};
	uint16_uint8_t ADC_brake_pressure_union_2 = {0};
	uint16_uint8_t ADC_brake_pressure_union_3 = {0};

	ADC_brake_pressure_union_1.uint16_num = incoming_data->adc_data.ADC_brake_pressure_1;
	ADC_brake_pressure_union_2.uint16_num = incoming_data->adc_data.ADC_brake_pressure_2;
	ADC_brake_pressure_union_3.uint16_num = incoming_data->adc_data.ADC_brake_pressure_3;

	uint8_t bp_data[CAN_DATA_LENGTH] = {ADC_brake_pressure_union_1.uint8_arr[0],
										ADC_brake_pressure_union_1.uint8_arr[1],
										ADC_brake_pressure_union_2.uint8_arr[0],
										ADC_brake_pressure_union_2.uint8_arr[1],
										ADC_brake_pressure_union_3.uint8_arr[0],
										ADC_brake_pressure_union_3.uint8_arr[1],
										0,
										0};


	HAL_CAN_AddTxMessage(&hcan1, &brake_and_steering_header, bp_data, &can_mailbox);
}


void CAN_SendVDSDiagnostic(VDS_Data_t *vds_status){

	// TODO: Once reqs have been decided, implementation here

}

void CAN_processMessages(void)
{
	//Update timers
	vds_data.currentTick_1 = HAL_GetTick();
	vds_data.currentTick_100 = HAL_GetTick();

	//Sum the incoming ADC values and convert them
	adc_averages.sum.ADC_brake_pressure_1 += CAN_Convert_BP_Value(vds_data.adc_data.ADC_brake_pressure_1);
	adc_averages.sum.ADC_brake_pressure_2 += CAN_Convert_BP_Value(vds_data.adc_data.ADC_brake_pressure_2);
	adc_averages.sum.ADC_brake_pressure_3 += CAN_Convert_BP_Value(vds_data.adc_data.ADC_brake_pressure_3);

	// Send brake and steering message at 10Hz (every 100ms)
	 if (vds_data.currentTick_100 - vds_data.previousTick_100 >= 100) {  // 100 ms has passed and CAN1 not in use
		 vds_data.previousTick_100 = vds_data.currentTick_100;

		 //Average the data
		 can_data.adc_data.ADC_brake_pressure_1 = adc_averages.sum.ADC_brake_pressure_1 / adc_averages.counter.ADC_brake_pressure_1;
		 can_data.adc_data.ADC_brake_pressure_2 = adc_averages.sum.ADC_brake_pressure_2 / adc_averages.counter.ADC_brake_pressure_2;
		 can_data.adc_data.ADC_brake_pressure_3 = adc_averages.sum.ADC_brake_pressure_3 / adc_averages.counter.ADC_brake_pressure_3;

		 CAN_SendBrakeAndSteering(&can_data);

		 // Reset the counters
		 adc_averages.counter.ADC_brake_pressure_1 = 0;
		 adc_averages.counter.ADC_brake_pressure_2 = 0;
		 adc_averages.counter.ADC_brake_pressure_3 = 0;
		 adc_averages.counter.ADC_steering_angle = 0;

		 // Reset the sums
		 adc_averages.sum.ADC_brake_pressure_1 = 0;
		 adc_averages.sum.ADC_brake_pressure_2 = 0;
		 adc_averages.sum.ADC_brake_pressure_3 = 0;
		 adc_averages.sum.ADC_steering_angle = 0;
	}

}

float CAN_Convert_BP_Value(uint16_t raw_adc_value){
	float adc_voltage = ((float) raw_adc_value / ADC_TO_VOLTAGE_DIVISOR) * (float)ADC_TO_VOLTAGE_MULTIPLIER;
	float mv_to_v_converted = (float)adc_voltage * MV_TO_V_MULTIPLIER;
	float pressure_value = (float)BRAKE_PRESSURE_MULTIPLIER * mv_to_v_converted;
	pressure_value += (float)BRAKE_PRESSURE_OFFSET;
	float pressure_per_surface_area = (float)pressure_value / BRAKE_PRESSURE_SURFACE_AREA;
	float pressure_in_psi = (float)pressure_per_surface_area * PSI_CONVERSION_FACTOR;

	return pressure_in_psi;
}

void CAN1_setBusyStatus(int flag_value)
{
  CAN1_DMA_busy_flag = flag_value;
}

void CAN2_setBusyStatus(int flag_value)
{
  CAN2_DMA_busy_flag = flag_value;
}

int CAN1_getBusyStatus(){
	return CAN1_DMA_busy_flag;
}


int CAN2_getBusyStatus(){
	return CAN2_DMA_busy_flag;
}


/* USER CODE END 0 */

CAN_HandleTypeDef hcan1;
CAN_HandleTypeDef hcan2;

/* CAN1 init function */
void MX_CAN1_Init(void)
{

  /* USER CODE BEGIN CAN1_Init 0 */

  /* USER CODE END CAN1_Init 0 */

  /* USER CODE BEGIN CAN1_Init 1 */

  /* USER CODE END CAN1_Init 1 */
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 8;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_4TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_4TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = DISABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = DISABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN1_Init 2 */

  /* USER CODE END CAN1_Init 2 */

}
/* CAN2 init function */
void MX_CAN2_Init(void)
{

  /* USER CODE BEGIN CAN2_Init 0 */

  /* USER CODE END CAN2_Init 0 */

  /* USER CODE BEGIN CAN2_Init 1 */

  /* USER CODE END CAN2_Init 1 */
  hcan2.Instance = CAN2;
  hcan2.Init.Prescaler = 8;
  hcan2.Init.Mode = CAN_MODE_NORMAL;
  hcan2.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan2.Init.TimeSeg1 = CAN_BS1_4TQ;
  hcan2.Init.TimeSeg2 = CAN_BS2_4TQ;
  hcan2.Init.TimeTriggeredMode = DISABLE;
  hcan2.Init.AutoBusOff = DISABLE;
  hcan2.Init.AutoWakeUp = DISABLE;
  hcan2.Init.AutoRetransmission = DISABLE;
  hcan2.Init.ReceiveFifoLocked = DISABLE;
  hcan2.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN2_Init 2 */

  /* USER CODE END CAN2_Init 2 */

}

static uint32_t HAL_RCC_CAN1_CLK_ENABLED=0;

void HAL_CAN_MspInit(CAN_HandleTypeDef* canHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspInit 0 */

  /* USER CODE END CAN1_MspInit 0 */
    /* CAN1 clock enable */
    HAL_RCC_CAN1_CLK_ENABLED++;
    if(HAL_RCC_CAN1_CLK_ENABLED==1){
      __HAL_RCC_CAN1_CLK_ENABLE();
    }

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**CAN1 GPIO Configuration
    PA11     ------> CAN1_RX
    PA12     ------> CAN1_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN CAN1_MspInit 1 */

  /* USER CODE END CAN1_MspInit 1 */
  }
  else if(canHandle->Instance==CAN2)
  {
  /* USER CODE BEGIN CAN2_MspInit 0 */

  /* USER CODE END CAN2_MspInit 0 */
    /* CAN2 clock enable */
    __HAL_RCC_CAN2_CLK_ENABLE();
    HAL_RCC_CAN1_CLK_ENABLED++;
    if(HAL_RCC_CAN1_CLK_ENABLED==1){
      __HAL_RCC_CAN1_CLK_ENABLE();
    }

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**CAN2 GPIO Configuration
    PB12     ------> CAN2_RX
    PB13     ------> CAN2_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN CAN2_MspInit 1 */

  /* USER CODE END CAN2_MspInit 1 */
  }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* canHandle)
{

  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspDeInit 0 */

  /* USER CODE END CAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    HAL_RCC_CAN1_CLK_ENABLED--;
    if(HAL_RCC_CAN1_CLK_ENABLED==0){
      __HAL_RCC_CAN1_CLK_DISABLE();
    }

    /**CAN1 GPIO Configuration
    PA11     ------> CAN1_RX
    PA12     ------> CAN1_TX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11|GPIO_PIN_12);

  /* USER CODE BEGIN CAN1_MspDeInit 1 */

  /* USER CODE END CAN1_MspDeInit 1 */
  }
  else if(canHandle->Instance==CAN2)
  {
  /* USER CODE BEGIN CAN2_MspDeInit 0 */

  /* USER CODE END CAN2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CAN2_CLK_DISABLE();
    HAL_RCC_CAN1_CLK_ENABLED--;
    if(HAL_RCC_CAN1_CLK_ENABLED==0){
      __HAL_RCC_CAN1_CLK_DISABLE();
    }

    /**CAN2 GPIO Configuration
    PB12     ------> CAN2_RX
    PB13     ------> CAN2_TX
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_12|GPIO_PIN_13);

  /* USER CODE BEGIN CAN2_MspDeInit 1 */

  /* USER CODE END CAN2_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
