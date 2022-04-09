/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "can.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/*
 *
 * Constant Definitions
 *
 *
 */
#define ADC_BUF_SIZE	12
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint16_t adc_data[ADC_BUF_SIZE]; // array to hold the temperature readings, and voltage and current readings
uint8_t conv_flag = 0; // flag to indicate when to convert the raw data from ADC to temperature
uint8_t idx = 0; // index for converting values in array
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

uint32_t ADC_VALUES[16] = {0};	// unprocessed ADC Values
double CONVERTED_VALUES[16] = {0};	// processed values

void Convert_Values(uint8_t index) {
	/* Convert value with appropriate relationship given the index */

	switch(index) {
		case 5:
			/* Relationship for converting ADC for Vsense 1 and modify CONVERTED_VALUES[index] */
			CONVERTED_VALUES[index] = (ADC_VALUES[index] + 82.621) / 40.271;	// slightly undercompensates
			break;
		case 6:
			/* Relationship for converting ADC for Vsense 1 and modify CONVERTED_VALUES[index] */
			CONVERTED_VALUES[index] = (ADC_VALUES[index] + 81.822) / 40.441;	// slightly overcompensates
			break;

		case 4:
		case 7:
			/* Relationship for converting to current and modify CONVERTED_VALUES[index] */

			break;

		case 0:
		case 1:
		case 2:
		case 3:
		case 8:
		case 9:
		case 14:
		case 15:
			/* Relationship for converting to temperature and modify CONVERTED_VALUES[index] */
			break;
	}
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_TIM1_Init();
  MX_CAN_Init();
  /* USER CODE BEGIN 2 */

  HAL_ADC_Start(&hadc1);


  /*
   *
   * Manually written code starts here
   *
   *
   */

  // Values from ADC; index 0 = voltage, 1 = current; 2 = raw temperature



//  if (HAL_OK !=  HAL_ADCEx_InjectedStart(&hadc1)) {
//	  Error_Handler();
//  }

  // if (HAL_OK != HAL_ADC_Start_DMA(&hadc1, temperatures, ADC_BUF_SIZE))
	  // Error_Handler();

  if (HAL_OK != HAL_TIM_Base_Start(&htim1)) {
	  Error_Handler();
  }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	  //code here
	  //read ADC, process, then output to CANbus

	  ADC_Select((uint8_t) 6);	// VSENSE1
	  HAL_ADC_PollForConversion(&hadc1, 1000);
	  ADC_VALUES[6] = HAL_ADC_GetValue(&hadc1);
	  Convert_Values((uint8_t) 6);

	  ADC_Select((uint8_t) 5); // VSENSE2
	  HAL_ADC_PollForConversion(&hadc1, 1000);
	  ADC_VALUES[5] = HAL_ADC_GetValue(&hadc1);
	  Convert_Values((uint8_t) 5);

	  ADC_Select((uint8_t) 7);	// ISENSE1
	  HAL_ADC_PollForConversion(&hadc1, 1000);
	  ADC_VALUES[7] = HAL_ADC_GetValue(&hadc1);

	  ADC_Select((uint8_t) 4);	// ISENSE2
	  HAL_ADC_PollForConversion(&hadc1, 1000);
	  ADC_VALUES[4] = HAL_ADC_GetValue(&hadc1);

	  ADC_Select((uint8_t) 0);	// TEMP_1
	  HAL_ADC_PollForConversion(&hadc1, 1000);
	  ADC_VALUES[0] = HAL_ADC_GetValue(&hadc1);

	  ADC_Select((uint8_t) 1); // TEMP_2
	  HAL_ADC_PollForConversion(&hadc1, 1000);
	  ADC_VALUES[1] = HAL_ADC_GetValue(&hadc1);

	  ADC_Select((uint8_t) 2);	// TEMP_3
	  HAL_ADC_PollForConversion(&hadc1, 1000);
	  ADC_VALUES[2] = HAL_ADC_GetValue(&hadc1);

	  ADC_Select((uint8_t) 3);	// TEMP_4
	  HAL_ADC_PollForConversion(&hadc1, 1000);
	  ADC_VALUES[3] = HAL_ADC_GetValue(&hadc1);

	  ADC_Select((uint8_t) 14);	// TEMP_5
	  HAL_ADC_PollForConversion(&hadc1, 1000);
	  ADC_VALUES[14] = HAL_ADC_GetValue(&hadc1);

	  ADC_Select((uint8_t) 15); // TEMP_6
	  HAL_ADC_PollForConversion(&hadc1, 1000);
	  ADC_VALUES[15] = HAL_ADC_GetValue(&hadc1);

	  ADC_Select((uint8_t) 8);	// TEMP_7
	  HAL_ADC_PollForConversion(&hadc1, 1000);
	  ADC_VALUES[8] = HAL_ADC_GetValue(&hadc1);

	  ADC_Select((uint8_t) 9);	// TEMP_8
	  HAL_ADC_PollForConversion(&hadc1, 1000);
	  ADC_VALUES[9] = HAL_ADC_GetValue(&hadc1);


	  HAL_Delay (1000);


  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	if(HAL_ADC_Stop_DMA(&hadc1) != HAL_OK)
		Error_Handler();

	conv_flag = 1; // flag set to 1 signals to the infinite while(1) loop to process the raw ADC data and convert to a temp value
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
