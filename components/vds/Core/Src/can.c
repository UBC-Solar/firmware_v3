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

void CAN_SendShockTravel(VDS_Data_t *vds_data)
{
    CAN_Message_t message;
    message.header = shock_travel_header;

    ADC_Value_t adc_value;
    uint16_t *shock_travel_ptr = (uint16_t*)&vds_data->adc_data.ADC_shock_travel_1;

    for (int i = 0; i < 4; ++i)
    {
        adc_value.value = *(shock_travel_ptr + i);
        message.data[i * 2] = adc_value.bytes.low;
        message.data[i * 2 + 1] = adc_value.bytes.high;
    }

    HAL_CAN_AddTxMessage(&hcan2, &message.header, message.data, &can_mailbox);
}


void CAN_SendBrakeAndSteering(VDS_Data_t *vds_data)
{
    CAN_Message_t message;
    message.header = brake_and_steering_header;

    ADC_Value_t adc_value;
    volatile uint16_t *sensor_data[] = {
        &vds_data->adc_data.ADC_brake_pressure_1,
        &vds_data->adc_data.ADC_brake_pressure_2,
        &vds_data->adc_data.ADC_brake_pressure_3,
        &vds_data->adc_data.ADC_steering_angle
    };

    for (int i = 0; i < 4; ++i)
    {
        adc_value.value = *(sensor_data[i]);
        message.data[i * 2] = adc_value.bytes.low;
        message.data[i * 2 + 1] = adc_value.bytes.high;
    }

    HAL_CAN_AddTxMessage(&hcan1, &message.header, message.data, &can_mailbox);
}


void CAN_SendVDSDiagnostic(VDS_Data_t *vds_status){

	// TODO: Once reqs have been decided, implementation here

}

void CAN_processMessages(void)
{

	float sensor_temps[NUM_ADC_CHANNELS_USED];
	float converted_values[NUM_ADC_CHANNELS_USED] = {0};

	converted_values[0] = (BRAKE_PRESSURE_MULTIPLIER * vds_data.adc_data.ADC_brake_pressure_1) + BRAKE_PRESSURE_OFFSET;
	converted_values[1] = (BRAKE_PRESSURE_MULTIPLIER * vds_data.adc_data.ADC_brake_pressure_2) + BRAKE_PRESSURE_OFFSET;
	converted_values[2] = (BRAKE_PRESSURE_MULTIPLIER * vds_data.adc_data.ADC_brake_pressure_3) + BRAKE_PRESSURE_OFFSET;
	converted_values[3] = vds_data.adc_data.ADC_steering_angle;
	converted_values[4] = (SHOCK_TRAVEL_MULTIPLIER * vds_data.adc_data.ADC_shock_travel_1) + SHOCK_TRAVEL_OFFSET;
	converted_values[5] = (SHOCK_TRAVEL_MULTIPLIER * vds_data.adc_data.ADC_shock_travel_2) + SHOCK_TRAVEL_OFFSET;
	converted_values[6] = (SHOCK_TRAVEL_MULTIPLIER * vds_data.adc_data.ADC_shock_travel_3) + SHOCK_TRAVEL_OFFSET;
	converted_values[7] = (SHOCK_TRAVEL_MULTIPLIER * vds_data.adc_data.ADC_shock_travel_4) + SHOCK_TRAVEL_OFFSET;


	for(int i = 0; i <= NUM_ADC_CHANNELS_USED - 1; i++){
		sensor_temps[i] += converted_values[i];
	}

	// Send brake and steering message at 10Hz (every 100ms)
	if (brake_steering_counter >= 100)
	{
		vds_data.adc_data.ADC_brake_pressure_1 = sensor_temps[0] / brake_steering_counter;
		vds_data.adc_data.ADC_brake_pressure_2 = sensor_temps[1] / brake_steering_counter;
		vds_data.adc_data.ADC_brake_pressure_3 = sensor_temps[2] / brake_steering_counter;
		vds_data.adc_data.ADC_steering_angle = sensor_temps[3] / brake_steering_counter;


		CAN_SendBrakeAndSteering(&vds_data);
		brake_steering_counter = 0; // Reset counter
	}

	// Send shock travel message at 1Khz
	if (shock_travel_counter >= 1){
		vds_data.adc_data.ADC_shock_travel_1 = sensor_temps[4] / shock_travel_counter;
		vds_data.adc_data.ADC_shock_travel_2 = sensor_temps[5] / shock_travel_counter;
		vds_data.adc_data.ADC_shock_travel_3 = sensor_temps[6] / shock_travel_counter;
		vds_data.adc_data.ADC_shock_travel_4 = sensor_temps[7] / shock_travel_counter;

		CAN_SendShockTravel(&vds_data);
		shock_travel_counter = 0;
	}

	//TODO: Implement VDS diagnostic once ready
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
