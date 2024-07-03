/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    adc.c
 * @brief   This file provides code for the configuration
 *          of the ADC instances.
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
#include "adc.h"

/* USER CODE BEGIN 0 */
#include <stdint.h>
#include "common.h"
/* USER CODE END 0 */

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

/* ADC1 init function */
void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T3_TRGO;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 8;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_2;
  sConfig.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_3;
  sConfig.Rank = ADC_REGULAR_RANK_4;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = ADC_REGULAR_RANK_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = ADC_REGULAR_RANK_6;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_6;
  sConfig.Rank = ADC_REGULAR_RANK_7;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_7;
  sConfig.Rank = ADC_REGULAR_RANK_8;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  if (HAL_ADCEx_Calibration_Start(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE END ADC1_Init 2 */

}

void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(adcHandle->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspInit 0 */

  /* USER CODE END ADC1_MspInit 0 */
    /* ADC1 clock enable */
    __HAL_RCC_ADC1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**ADC1 GPIO Configuration
    PA0-WKUP     ------> ADC1_IN0
    PA1     ------> ADC1_IN1
    PA2     ------> ADC1_IN2
    PA3     ------> ADC1_IN3
    PA4     ------> ADC1_IN4
    PA5     ------> ADC1_IN5
    PA6     ------> ADC1_IN6
    PA7     ------> ADC1_IN7
    */
    GPIO_InitStruct.Pin = ST_1_Pin|ST_2_Pin|ST_3_Pin|ST_4_Pin
                          |BP_1_Pin|BP_2_Pin|BP_3_Pin|SA_1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* ADC1 DMA Init */
    /* ADC1 Init */
    hdma_adc1.Instance = DMA1_Channel1;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc1.Init.Mode = DMA_CIRCULAR;
    hdma_adc1.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_adc1) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(adcHandle,DMA_Handle,hdma_adc1);

    /* ADC1 interrupt Init */
    HAL_NVIC_SetPriority(ADC1_2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(ADC1_2_IRQn);
  /* USER CODE BEGIN ADC1_MspInit 1 */

  /* USER CODE END ADC1_MspInit 1 */
  }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle)
{

  if(adcHandle->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspDeInit 0 */

  /* USER CODE END ADC1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_ADC1_CLK_DISABLE();

    /**ADC1 GPIO Configuration
    PA0-WKUP     ------> ADC1_IN0
    PA1     ------> ADC1_IN1
    PA2     ------> ADC1_IN2
    PA3     ------> ADC1_IN3
    PA4     ------> ADC1_IN4
    PA5     ------> ADC1_IN5
    PA6     ------> ADC1_IN6
    PA7     ------> ADC1_IN7
    */
    HAL_GPIO_DeInit(GPIOA, ST_1_Pin|ST_2_Pin|ST_3_Pin|ST_4_Pin
                          |BP_1_Pin|BP_2_Pin|BP_3_Pin|SA_1_Pin);

    /* ADC1 DMA DeInit */
    HAL_DMA_DeInit(adcHandle->DMA_Handle);

    /* ADC1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(ADC1_2_IRQn);
  /* USER CODE BEGIN ADC1_MspDeInit 1 */

  /* USER CODE END ADC1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

// Helper function to calculate the average of non-zero elements in an array
float getAveragedValue(volatile uint16_t array[], uint16_t length)
{
    uint32_t sum = 0;   // Sum of non-zero elements
    uint16_t count = 0; // Count of non-zero elements

    for (uint16_t i = 0; i < length; i++)
    {
        if (array[i] != 0)
        {
            sum += array[i];
            count++;
        }
    }

    // If there are no non-zero elements, return 0 to avoid division by zero
    if (count == 0)
    {
        return 0.0f;
    }

    return (float)sum / count;
}


/**
 * @brief Converts the raw readings of ADC1 into ADC voltage readings and
 * averages these values at half of its buffer size per channel,
 * when the DMA interrupt for ADC1 is called when half of its scan
 * conversion sequence is completed. These output values will then be converted
 * into VDS data variables.
 *
 * @param half Integer (1 or 0) indicating if the averaging is from the first half (0)
 *             of its circular buffer or its second half (1).
 * @param adc1_buf Volatile uint16_t array that represents ADC1's DMA circular buffer.
 *                 Each time the ADC is read, the next 4 elements get populated
 *                 with the ADC readings for each channel in the following order:
 *                 ADC_brake_pressure_1, ADC_brake_pressure_2, ADC_brake_pressure_3,
 *                 ADC_shock_travel_1, ADC_shock_travel_2, ADC_shock_travel_3,
 *                 ADC_shock_travel_4, ADC_steering_angle.
 * @param result Float array of size 8, storing the averaged ADC voltage readings for
 *               brake and steering angle.
 * @retval None.
 */
void ADC1_processRawReadings(int half, volatile uint16_t adc1_buf[], float result[])
{
	ADC1_setBusyStatus(1);
	int start_index = half * (ADC1_BUF_LENGTH >> 1); // Start of the selected half buffer

	// Update the Shared VDS struct with values from the first half of the buffer
	vds_data.adc_data.ADC_brake_pressure_1 = adc1_buf[start_index + BP_1];
	vds_data.adc_data.ADC_brake_pressure_2 = adc1_buf[start_index + BP_2];
	vds_data.adc_data.ADC_brake_pressure_3 = adc1_buf[start_index + BP_3];
	vds_data.adc_data.ADC_shock_travel_1 = adc1_buf[start_index + ST_1];
	vds_data.adc_data.ADC_shock_travel_2 = adc1_buf[start_index + ST_2];
	vds_data.adc_data.ADC_shock_travel_3 = adc1_buf[start_index + ST_3];
	vds_data.adc_data.ADC_shock_travel_4 = adc1_buf[start_index + ST_4];
	vds_data.adc_data.ADC_steering_angle = adc1_buf[start_index + SA_1];

	ADC1_setBusyStatus(0);
}

/**
 * @brief sets the Fault flag in the global variable ADC1_DMA_fault_flag
 * depending if ADC1 attempts to read values in the middle of a DMA interrupt callback process
 *
 * @param flag_value integer value: 1 is at fault, 0 is not at fault
 * @retval sets ADC1_DMA_fault_flag with flag_value
 */
void ADC1_setFaultStatus(int flag_value)
{
  ADC1_DMA_fault_flag = flag_value;
}

/**
 * @brief Retrieves the fault status of ADC1, stored in global variable
 * ADC1_DMA_fault_flag
 *
 * @param -
 * @retval returns the global variable ADC1_DMA_fault_flag (int datatype).
 *         1 means at fault; 0 means not at fault
 */
int ADC1_getFaultStatus()
{
  return ADC1_DMA_fault_flag;
}

/**
 * @brief sets the busy flag in the global variable ADC1_DMA_in_process_flag
 * depending if ADC1 attempts to read values in the middle of a DMA interrupt callback process
 *
 * @param flag_value integer value: 1 is busy, 0 is not at busy
 * @retval sets ADC1_DMA_in_process_flag with flag_value
 */
void ADC1_setBusyStatus(int flag_value)
{
  ADC1_DMA_in_process_flag = flag_value;
}

/**
 * @brief Retrieves the busy status of ADC1, stored in global variable
 * ADC1_DMA_in_process_flag
 *
 * @param -
 * @retval returns the global variable ADC1_DMA_in_process_flag (int datatype).
 *         1 means at busy; 0 means not at busy
 */
int ADC1_getBusyStatus()
{
  return ADC1_DMA_in_process_flag;
}

/* USER CODE END 1 */
