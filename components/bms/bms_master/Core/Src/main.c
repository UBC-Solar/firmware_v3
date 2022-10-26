/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define LED_BLINK_INTERVAL 500 // milliseconds
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CAN_HandleTypeDef hcan;

SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

BTM_PackData_t pack;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CAN_Init(void);
static void MX_SPI2_Init(void);
static void MX_TIM3_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */
uint8_t doesRegGroupMatch(uint8_t reg_group1[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE],
                          uint8_t reg_group2[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE]);
void packUpdateAndControl(BTM_PackData_t *pack);
void startupChecks(BTM_PackData_t *pack);
void analyzePack(BTM_PackData_t *pack);
void stopBalancing(BTM_PackData_t *pack);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
  /* USER CODE BEGIN 1 */

  uint8_t test_data[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE] = {{0x55, 0x6E, 0x69, 0x42, 0x43, 0x20}, {0x53, 0x6f, 0x6c, 0x61, 0x72, 0x21}};
  uint8_t test_data_rx[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE] = {0};
  BTM_Status_t comm_status = {BTM_OK, 0};
  uint8_t startup_flag = 1; // indicates the BMS has been powered on, need to init relevant parameters
  uint8_t reg_group_match_failed = 0;
  unsigned int current_blink_tick = 0;
  unsigned int last_blink_tick = 0;
  unsigned int last_measurement_tick = HAL_GetTick();
  unsigned int last_CAN_tick = HAL_GetTick();
  unsigned int current_tick;

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
  MX_CAN_Init();
  MX_SPI2_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  DebugIO_Init(&huart1);

  // Give the BMS functionality access to hardware resources
  BTM_SPI_handle = &hspi2;
  CONT_timer_handle = &htim3;
  // TODO: CAN handle

  HAL_GPIO_WritePin(LED_OUT_GPIO_Port, LED_OUT_Pin, GPIO_PIN_SET); // Turn LED on
  current_blink_tick = HAL_GetTick();

  // Initialize hardware and pack struct
  CONT_init();     // control signals
  BTM_init(&pack); // initialize the LTC6813 and system data
  // TODO: CAN initilization

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    current_tick = HAL_getTick();

    // on startup, preform system checks
    if (startup_flag)
    {
      startupChecks(&pack);
    }

    // update pack values and control signals if pack update interval has elapsed
    if (current_tick - last_measurement_tick >= MEASUREMENT_INTERVAL)
    {
      packUpdateAndControl(&pack);
    }

    // preform CAN communication if CAN send interval has elapsed
    if (current_tick - last_measurement_tick >= CAN_TX_INTERVAL)
    {
      // TODO: CAN communication here (always happens, no matter whether we're normal or fault)
    }

    // blink LED on master board
    current_blink_tick = HAL_GetTick();
    if (current_blink_tick - last_blink_tick >= LED_BLINK_INTERVAL)
    {
      HAL_GPIO_TogglePin(LED_OUT_GPIO_Port, LED_OUT_Pin);
      last_blink_tick = current_blink_tick;
    }

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

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_RCC_MCOConfig(RCC_MCO, RCC_MCO1SOURCE_PLLCLK, RCC_MCODIV_1);

  /** Enables the Clock Security System
   */
  HAL_RCC_EnableCSS();
}

/**
 * @brief CAN Initialization Function
 * @param None
 * @retval None
 */
static void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN1;
  hcan.Init.Prescaler = 18;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_12TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_3TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = DISABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = DISABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN_Init 2 */

  /* USER CODE END CAN_Init 2 */
}

/**
 * @brief SPI2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi2.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */
}

/**
 * @brief TIM3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 7;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 359;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);
}

/**
 * @brief USART1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, OT_OUT_Pin | BAL_OUT_Pin | FLT_OUT_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, LLIM_OUT_Pin | HLIM_OUT_Pin | LED_OUT_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, COM_OUT_Pin | SPI_LTC_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : GPIO_OUT_Pin */
  GPIO_InitStruct.Pin = GPIO_OUT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  HAL_GPIO_Init(GPIO_OUT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : OT_OUT_Pin BAL_OUT_Pin FLT_OUT_Pin */
  GPIO_InitStruct.Pin = OT_OUT_Pin | BAL_OUT_Pin | FLT_OUT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : LLIM_OUT_Pin HLIM_OUT_Pin LED_OUT_Pin */
  GPIO_InitStruct.Pin = LLIM_OUT_Pin | HLIM_OUT_Pin | LED_OUT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : COM_OUT_Pin */
  GPIO_InitStruct.Pin = COM_OUT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(COM_OUT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SPI_LTC_CS_Pin */
  GPIO_InitStruct.Pin = SPI_LTC_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(SPI_LTC_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PA8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/*============================================================================*/
/* PRIVATE FUNCTIONS */

/**
 * @brief
 *
 *
 */

void packUpdateAndControl(BTM_PackData_t *pack)
{
  unsigned int fan_PWM = 0;
  float max_temp = 0;
  int32_t pack_current;
  uint8_t DOC_COC_active = 0;
  unsigned int current_tick;
  static uint8_t initial_soc_measurement_flag = 1;
  unsigned int HLIM_status = 0;

  current_tick = HAL_getTick();

  // TODO: recieve ECU CAN message

  if (DOC_COC_active)
  {
    pack->status |= FLT_DOC_COC_MASK;
  }

  // get measurements
  if (BTM_readBatt(pack).error != BTM_OK || BTM_TEMP_measureState(pack).error != BTM_OK)
  {
    pack->status |= FLT_COMM_MASK;
  }

  // SOC estimation
  if (initial_soc_measurement_flag) // only preform initilization of SOC once (at startup)
  {
    SOC_allModulesInit(pack);
    initial_soc_measurement_flag = 0;
  }
  else
  {
    SOC_allModulesEst(pack, pack_current, current_tick);
  }

  // write status code
  analyzePack(pack);

  // if fault, drive FLT, COM, OT GPIOs, turn off balancing, drive fans 100%
  if (pack->status & FAULTS_MASK)
  {
    CONT_FLT_switch((pack->status & FAULTS_MASK) ? CONT_ACTIVE : CONT_INACTIVE);
    CONT_COM_switch((pack->status & FLT_COMM_MASK) ? CONT_ACTIVE : CONT_INACTIVE);
    CONT_OT_switch((pack->status & FLT_OT_MASK) ? CONT_ACTIVE : CONT_INACTIVE);
    stopBalancing(pack);
    CONT_BAL_switch((pack->status & TRIP_BAL_MASK) ? CONT_ACTIVE : CONT_INACTIVE);
    CONT_FAN_PWM_set(FAN_FULL);
  }
  else // no fault, balance, drive control signals, drive fans based on temp
  {
    BTM_BAL_settings(pack);
    ANA_writePackBalStatus(pack);
    CONT_BAL_switch((pack->status & TRIP_BAL_MASK) ? CONT_ACTIVE : CONT_INACTIVE);

    // OR together HLIM and CHARGE_OT status bits, HLIM active if either are active
    HLIM_status = (pack->status & TRIP_HLIM_MASK) | (pack->status & TRIP_CHARGE_OT_MASK);
    CONT_HLIM_switch(HLIM_status ? CONT_ACTIVE : CONT_INACTIVE);
    CONT_LLIM_switch((pack->status & TRIP_LLIM_MASK) ? CONT_ACTIVE : CONT_INACTIVE);

    max_temp = ANA_findHighestModuleTemp(pack);
    fan_PWM = CONT_fanPwmFromTemp(max_temp);
    CONT_FAN_PWM_set(fan_PWM);
  }
}

/**
 * @brief Preforms initial system checks
 * Writes COMM bit in status code if checks fail
 */

void startupChecks(BTM_PackData_t *pack)
{
  uint8_t test_data[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE] = {{0x55, 0x6E, 0x69, 0x42, 0x43, 0x20}, {0x53, 0x6f, 0x6c, 0x61, 0x72, 0x21}};
  uint8_t test_data_rx[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE] = {0};
  BTM_Status_t comm_status = {BTM_OK, 0};
  uint8_t reg_group_match_failed = 0;

  BTM_writeRegisterGroup(CMD_WRCOMM, test_data);
  comm_status = BTM_readRegisterGroup(CMD_RDCOMM, test_data_rx);
  reg_group_match_failed = doesRegGroupMatch(test_data, test_data_rx);

  // checks for comms error
  if (reg_group_match_failed || comm_status.error != BTM_OK)
  {
    pack->status |= FLT_COMM_MASK;
  }
}

/**
 * @brief Helper function for initial system checks
 * Returns 0 for full match, 1 otherwise
 */

uint8_t doesRegGroupMatch(uint8_t reg_group1[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE],
                          uint8_t reg_group2[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE])
{
  for (int ic_num = 0; ic_num < BTM_NUM_DEVICES; ic_num++)
  {
    for (int i = 0; i < BTM_REG_GROUP_SIZE; i++)
    {
      if (reg_group1[ic_num][i] != reg_group2[ic_num][i])
        return 1;
    }
  }
  return 0;
}

/**
 * @brief Helper function to write the pack status code
 * Merges module status codes into the pack status code
 */

void analyzePack(BTM_PackData_t *pack)
{
  unsigned int status;
  ANA_analyzeModules(pack);
  status = ANA_mergeModuleStatusCodes(pack);
  pack->status = status;
}

/**
 * @brief Helper function to stop balancing for all modules
 *
 */

void stopBalancing(BTM_PackData_t *pack)
{
  for (int ic_num = 0; ic_num < BTM_NUM_DEVICES; ic_num++)
  {
    for (int module_num = 0; module_num < BTM_NUM_MODULES; module_num++)
    {
      pack->stack[ic_num].module[module_num].bal_status = DISCHARGE_OFF;
    }
  }

  BTM_BAL_setDischarge(pack);     // will clear bal module status
  pack->status &= ~TRIP_BAL_MASK; // clear bal pack status
}

/**
 * @brief Configure where printf() and putchar() output goes
 */
int __io_putchar(int ch)
{
  // Un-comment either (or both) function call to route debugging output

  // Output on Serial Wire Output (SWO)
  ITM_SendChar(ch);

  // Output on UART
  // HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);

  return (ch);
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

  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
