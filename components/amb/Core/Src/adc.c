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
  hadc1.Init.ContinuousConvMode = ENABLE;
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

  //Very useful function for calibrating ADC before usage. Always include this
  if (HAL_ADCEx_Calibration_Start(&hadc1) != HAL_OK){
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
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**ADC1 GPIO Configuration
    PA0-WKUP     ------> ADC1_IN0
    PA1     ------> ADC1_IN1
    PA2     ------> ADC1_IN2
    PA3     ------> ADC1_IN3
    PA4     ------> ADC1_IN4
    PA5     ------> ADC1_IN5
    PA6     ------> ADC1_IN6
    PA7     ------> ADC1_IN7
    PC4     ------> ADC1_IN14
    PC5     ------> ADC1_IN15
    PB0     ------> ADC1_IN8
    PB1     ------> ADC1_IN9
    */
    GPIO_InitStruct.Pin = TEMP_1_Pin|TEMP_2_Pin|TEMP_3_Pin|TEMP_4_Pin
                          |ISENSE2_Pin|VSENSE2_Pin|VSENSE1_Pin|ISENSE1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = TEMP_5_Pin|TEMP_6_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = TEMP_7_Pin|TEMP_8_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

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
    PC4     ------> ADC1_IN14
    PC5     ------> ADC1_IN15
    PB0     ------> ADC1_IN8
    PB1     ------> ADC1_IN9
    */
    HAL_GPIO_DeInit(GPIOA, TEMP_1_Pin|TEMP_2_Pin|TEMP_3_Pin|TEMP_4_Pin
                          |ISENSE2_Pin|VSENSE2_Pin|VSENSE1_Pin|ISENSE1_Pin);

    HAL_GPIO_DeInit(GPIOC, TEMP_5_Pin|TEMP_6_Pin);

    HAL_GPIO_DeInit(GPIOB, TEMP_7_Pin|TEMP_8_Pin);

  /* USER CODE BEGIN ADC1_MspDeInit 1 */

  /* USER CODE END ADC1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* Below are functions to select between ADC Channels
 * sConfig.Channel is set to the appropriate channel required
 * https://controllerstech.com/stm32-adc-multi-channel-without-dma/
 */


/* Function takes channel as argument to set configuration */

void ADC_Select(uint8_t channel)
{
	ADC_ChannelConfTypeDef sConfig = {0};

	switch(channel) {
		case TEMP_1:		// TEMP_1
			sConfig.Channel = ADC_CHANNEL_0;
			break;
		case TEMP_2:		// TEMP_2
			sConfig.Channel = ADC_CHANNEL_1;
			break;
		case TEMP_3:		// TEMP_3
			sConfig.Channel = ADC_CHANNEL_2;
			break;
		case TEMP_4:		// TEMP_4
			sConfig.Channel = ADC_CHANNEL_3;
			break;
		case ISENSE2:		// ISENSE2
			sConfig.Channel = ADC_CHANNEL_4;
			break;
		case VSENSE2:		// VSENSE2
			sConfig.Channel = ADC_CHANNEL_5;
			break;
		case VSENSE1:		// VSENSE1
			sConfig.Channel = ADC_CHANNEL_6;
			break;
		case ISENSE1:		// ISENSE1
			sConfig.Channel = ADC_CHANNEL_7;
			break;
		case TEMP_7:		// TEMP_7
			sConfig.Channel = ADC_CHANNEL_8;
			break;
		case TEMP_8:		// TEMP_8
			sConfig.Channel = ADC_CHANNEL_9;
			break;
		case 10:	// NOT IN USE
			//sConfig.Channel = ADC_CHANNEL_10;
			break;
		case 11:	// NOT IN USE
			//sConfig.Channel = ADC_CHANNEL_11;
			break;
		case 12:	// NOT IN USE
			//sConfig.Channel = ADC_CHANNEL_12;
			break;
		case 13:	// NOT IN USE
			//sConfig.Channel = ADC_CHANNEL_13;
			break;
		case TEMP_5:	// TEMP_5
			sConfig.Channel = ADC_CHANNEL_14;
			break;
		case TEMP_6:	// TEMP_6
			sConfig.Channel = ADC_CHANNEL_15;
			break;
	}

	sConfig.Rank = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES;	// may need to change?

	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
	{
		Error_Handler();
	}
}

/* Below is code that can be deleted once the above ADC code is complete.
 * Just keep it for now for reference to how to set up the channel for reading
 */

/* Voltage Channels*/

///* vsense 1 = channel 5*/
//void ADC_Select_CH5(void)
//{
//	ADC_ChannelConfTypeDef sConfig = {0};
//	/*Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. */
//	sConfig.Channel = ADC_CHANNEL_5;
//	sConfig.Rank = 1;
//	// Not sure about below sampling time
//	sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES;	// defined above
//	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
//	{
//		Error_Handler();
//	}
//}

///* vsense2 = channel 6 */
//void ADC_Select_CH6(void)
//{
//	ADC_ChannelConfTypeDef sConfig = {0};
//	/*Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. */
//	sConfig.Channel = ADC_CHANNEL_6;
//	sConfig.Rank = 1;
//	// Not sure about below sampling time
//	sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES;	// defined above
//	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
//	{
//		Error_Handler();
//	}
//
//}
//
///* Current Channel */
//void ADC_Select_CH4(void)
//{
//	ADC_ChannelConfTypeDef sConfig = {0};
//	/* Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. */
//	sConfig.Channel = ADC_CHANNEL_4;
//	sConfig.Rank = 1;
//	// Not sure about below Sampling Time
//	sConfig.SamplingTime = ADC_SAMPLETIME_84CYCLES;	// defined above
//	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
//	{
//		Error_Handler();
//	}
//}
//
///* Temperature Channel */
//void ADC_Select_CH2(void)
//{
//	ADC_ChannelConfTypeDef sConfig = {0};
//	/* Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. */
//	sConfig.Channel = ADC_CHANNEL_2;
//	sConfig.Rank = 1;
//	// Not sure about below sampling time
//	sConfig.SamplingTime = ADC_SAMPLETIME_112CYCLES;		// defined above
//	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
//	{
//		Error_Handler();
//	}
//}

/* USER CODE END 1 */
