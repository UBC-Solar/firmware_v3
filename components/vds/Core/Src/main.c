/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "adc.h"
#include "can.h"
#include "dma.h"
#include "iwdg.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "common.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
volatile uint16_t adc1_buf[NUM_ADC_CHANNELS_TOTAL] = {0}; // Initialize ADC buffer
VDS_Data_t vds_data = {0}; // Initialize VDS data structure

static uint32_t last_brake_and_steering_time = 0;
static uint32_t last_shock_travel_time = 0;
//static uint32_t last_diagnostic_time = 0;

//Initialise global variables for the VDS data structure and ADC readings
volatile int ADC1_DMA_in_process_flag = 0; //flag that indicates the DMA interrupt if ADC1 has been called and is in process
volatile int ADC1_DMA_fault_flag = 0; //flag that indicates the DMA interrupt if ADC1 has been called and is at fault

//keeping track of averages
float sum[NUM_ADC_CHANNELS_USED] = {0.0};
uint32_t counters[NUM_ADC_CHANNELS_USED] = {0};
volatile VDS_ADC_AVERAGES adc_averages = {0};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void averageADCValues(int adc_half);
void processCANMessages(void);
void convertADCValue(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_ADC_LevelOutOfWindowCallback(ADC_HandleTypeDef *hadc)
{ // Analog watchdog 
  /*TODO: Implement error handling here */
  if(hadc == &hadc1)
  {
    ADC1_setFaultStatus(1);
    HAL_Delay(1); // Delay for 1ms
  }
}

/*============================================================================*/
/* GPIO INTERRUPT CALLBACKS */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  /*TODO: Implement error handling here */  
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  HAL_Delay(100); // Delay for 100ms to allow peripherals to initialize

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  //Reset all adc flags
  vds_data.status.raw = 0;
  vds_data.adc_data.ADC_brake_pressure_1 = 0;
  vds_data.adc_data.ADC_brake_pressure_2 = 0;
  vds_data.adc_data.ADC_brake_pressure_3 = 0;
  vds_data.adc_data.ADC_shock_travel_1 = 0;
  vds_data.adc_data.ADC_shock_travel_2 = 0;
  vds_data.adc_data.ADC_shock_travel_3 = 0;
  vds_data.adc_data.ADC_shock_travel_4 = 0;
  vds_data.adc_data.ADC_steering_angle = 0;
  vds_data.status.bits.adc_fault = 0;

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_CAN1_Init();
  MX_CAN2_Init();
  MX_ADC1_Init();
  MX_TIM3_Init();
  MX_IWDG_Init();
  /* USER CODE BEGIN 2 */
  HAL_CAN_Start(&hcan1);
  HAL_CAN_Start(&hcan2);
  HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc1_buf, NUM_ADC_CHANNELS_TOTAL); // Start ADC1 in DMA mode
  HAL_TIM_Base_Start(&htim3);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  //Independent watchdog timer and
	  HAL_IWDG_Refresh(&hiwdg);
	  convertADCValue();
	  processCANMessages();
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.Prediv1Source = RCC_PREDIV1_SOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  RCC_OscInitStruct.PLL2.PLL2State = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the Systick interrupt time
  */
  __HAL_RCC_PLLI2S_ENABLE();
}

/* USER CODE BEGIN 4 */

/*
Notes on how to do conversion:
1: lower half of ADC 
2: upper half of ADC

-pass which half needs to be processed as a parameter to processing methods
-convert raw ADC values to physical values
-assign physical values to the correct sensor in the VDS data structure
-handover the VDS data structure to the CAN module (to be handled by Diego)

*/

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
{
  if (hadc == &hadc1){
    // TODO: Process ADC1 readings
    averageADCValues(0);
  }
}

// Conversion full complete DMA interrupt callback for ADCs
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
  if (hadc == &hadc1){
    // TODO: Process ADC1 readings
    averageADCValues(1);
  }
}

void averageADCValues(int adc_half)
{ //TODO: Modify this function to process and save ADC1 readings for VDS

  if (!ADC1_getBusyStatus())
  {
    ADC1_setBusyStatus(1);
    static float result[NUM_ADC_CHANNELS_TOTAL]; //stores averaged results
    ADC1_processRawReadings(adc_half, adc1_buf, result); //function to process values from ADC1 and update vds struct
    ADC1_setBusyStatus(0);
  }
  else
  {
    ADC1_setFaultStatus(1);
    HAL_Delay(1); // Delay for 1ms
    averageADCValues(adc_half); // Retry
  }
}

//ADC error callback
void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
  if (hadc == &hadc1)
  {
    ADC1_setFaultStatus(1);
    HAL_Delay(1); // Delay for 1ms
    averageADCValues(0); // Retry with ADC half 0
    averageADCValues(1); // Retry with ADC half 1
  }
}

void convertADCValue(void){

  vds_data.adc_data.ADC_brake_pressure_1 = (24.55 * vds_data.adc_data.ADC_brake_pressure_1) + 1.925;
  vds_data.adc_data.ADC_brake_pressure_2 = (24.55 * vds_data.adc_data.ADC_brake_pressure_2) + 1.925;
  vds_data.adc_data.ADC_brake_pressure_3 = (24.55 * vds_data.adc_data.ADC_brake_pressure_3) + 1.925;
  vds_data.adc_data.ADC_shock_travel_1 = (0.1595 * vds_data.adc_data.ADC_shock_travel_1) + 2.3875;
  vds_data.adc_data.ADC_shock_travel_2 = (0.1595 * vds_data.adc_data.ADC_shock_travel_2) + 2.3875;
  vds_data.adc_data.ADC_shock_travel_3 = (0.1595 * vds_data.adc_data.ADC_shock_travel_3) + 2.3875;
  vds_data.adc_data.ADC_shock_travel_4 = (0.1595 * vds_data.adc_data.ADC_shock_travel_4) + 2.3875;

  // TODO: Add conversion for steering angle once sensor has been selected and tested

}

void processCANMessages(void)
{
  uint32_t current_time = HAL_GetTick();


  // Send shock travel message at 1kHz
  if (current_time - last_shock_travel_time >= 1)
  {
    CAN_SendShockTravel(&vds_data);
    last_shock_travel_time = current_time;
  }

  // Send brake and steering message at 10Hz
  if (current_time - last_brake_and_steering_time >= 100)
  {
    CAN_SendBrakeAndSteering(&vds_data);
    last_brake_and_steering_time = current_time;
  }

  // Send diagnostic message every 5 seconds
  // TODO: Determine what we want from and how to implement diagnostic message
//  if (current_time - last_diagnostic_time >= 5000)
//  {
//    CAN_SendDiagnostics(&vds_status);
//    last_diagnostic_time = current_time;
//  }
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
