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


// Defined values from documentation below
// https://www.disca.upv.es/aperles/arm_cortex_m3/llibre/st/STM32F439xx_User_Manual/group__adc__sampling__times.html
#define ADC_SAMPLETIME_28CYCLES ((uint32_t)ADC_SMPR1_SMP10_1)
#define ADC_SAMPLETIME_84CYCLES ((uint32_t)ADC_SMPR1_SMP10_2)
#define ADC_SAMPLETIME_112CYCLES ((uint32_t)(ADC_SMPR1_SMP10_2 | ADC_SMPR1_SMP10_0))
/* USER CODE END 0 */

ADC_HandleTypeDef hadc1;

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
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
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
    */
    GPIO_InitStruct.Pin = ST_1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(ST_1_GPIO_Port, &GPIO_InitStruct);

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
    */
    HAL_GPIO_DeInit(ST_1_GPIO_Port, ST_1_Pin);

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

//testing
void ADC_Select(uint8_t channel)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    switch(channel) {
        case BP_1:
            sConfig.Channel = ADC_CHANNEL_0;
            break;
        case BP_2:
            sConfig.Channel = ADC_CHANNEL_1;
            break;
        case BP_3:
            sConfig.Channel = ADC_CHANNEL_2;
            break;
        case ST_1:
            sConfig.Channel = ADC_CHANNEL_3;
            break;
        case ST_2:
            sConfig.Channel = ADC_CHANNEL_4;
            break;
        case ST_3:
            sConfig.Channel = ADC_CHANNEL_5;
            break;
        case ST_4:
            sConfig.Channel = ADC_CHANNEL_6;
            break;
        case SA_1:
            sConfig.Channel = ADC_CHANNEL_7;
            break;
        default:
            // Handle cases where channel does not correspond to a valid sensor
            // You may want to add error handling or a default case here
            break;
    }

    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES;

    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        Error_Handler();  // Replace with your error handling function
    }
}



// Function to poll and read ADC values for all sensors and update vds_data
void read_all_sensors(void) {
    uint16_t ADC_VALUES[8];   // Adjust size based on your sensor count

    // Iterate through each sensor and read its ADC value
    for (uint8_t sensor = 0; sensor < 8; sensor++) {
        ADC_Select(sensor);  // Select ADC channel for the sensor

        HAL_ADC_PollForConversion(&hadc1, 1000);  // Poll for ADC conversion
        ADC_VALUES[sensor] = HAL_ADC_GetValue(&hadc1);  // Get ADC value and store it
    }

    // Update vds_data struct with ADC values
    vds_data.adc_data.ADC_brake_pressure_1 = ADC_VALUES[BP_1];
    vds_data.adc_data.ADC_brake_pressure_2 = ADC_VALUES[BP_2];
    vds_data.adc_data.ADC_brake_pressure_3 = ADC_VALUES[BP_3];
    vds_data.adc_data.ADC_shock_travel_1 = ADC_VALUES[ST_1];
    vds_data.adc_data.ADC_shock_travel_2 = ADC_VALUES[ST_2];
    vds_data.adc_data.ADC_shock_travel_3 = ADC_VALUES[ST_3];
    vds_data.adc_data.ADC_shock_travel_4 = ADC_VALUES[ST_4];
    vds_data.adc_data.ADC_steering_angle = ADC_VALUES[SA_1];
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
