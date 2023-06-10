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
#include "ltc6813_btm.h"
#include "ltc6813_btm_temp.h"
#include "ltc6813_btm_bal.h"
#include "analysis.h"
#include "balancing.h"
#include "can.h"
#include "control.h"
#include "soc.h"
// #include "selftest.h"
#include "pack.h"
#include "debug_io.h"
#include <stdbool.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define LED_BLINK_INTERVAL 500 // milliseconds
#define MEASUREMENT_INTERVAL 1000 // milliseconds
#define CAN_TX_INTERVAL 1000 // milliseconds
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

Pack_t pack;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CAN_Init(void);
static void MX_SPI2_Init(void);
static void MX_TIM3_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */
bool doesRegGroupMatch(uint8_t reg_group1[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE],
                       uint8_t reg_group2[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE]);
void packUpdateAndControl(Pack_t *pack);
void startupChecks(Pack_t *pack);
void analyzePack(Pack_t *pack);
void stopBalancing(Pack_t *pack);

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
  uint32_t current_blink_tick = 0;
  uint32_t last_blink_tick = 0;
  uint32_t last_measurement_tick = 0;
  uint32_t last_CAN_tick = 0;
  uint32_t current_tick;

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

  CONT_timer_handle = &htim3;
  // TODO: CAN handle

  HAL_GPIO_WritePin(LED_OUT_GPIO_Port, LED_OUT_Pin, GPIO_PIN_SET); // Turn LED on
  current_blink_tick = HAL_GetTick();

  // Initialize hardware and pack struct
  CONT_init(); // control signals
  BTM_init(&hspi2); // initialize the LTC6813s and driver state
  // TODO: CAN initialization

  startupChecks(&pack);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    current_tick = HAL_GetTick();

    // update pack values and control signals if pack update interval has elapsed
    if (current_tick - last_measurement_tick >= MEASUREMENT_INTERVAL)
    {
      packUpdateAndControl(&pack);
      last_measurement_tick = current_tick;
    }

    // preform CAN communication if CAN send interval has elapsed
    if (current_tick - last_CAN_tick >= CAN_TX_INTERVAL)
    {
      // TODO: preform CAN communication
      // CAN comms always happen (whether or not in fault state)
      last_CAN_tick = current_tick;
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
 * @brief TODO:
 *
 *
 */
void packUpdateAndControl(Pack_t *pack)
{
  // Temperature
  uint32_t fan_PWM = 0;
  float max_temp = 0;
  // ECU CAN Message
  CAN_Rx_Message_t rxMessages[NUM_RX_FIFOS * MAX_MESSAGES_PER_FIFO];
  int32_t pack_current = 0;
  uint8_t DOC_COC_active = 0;
  CAN_Rx_Message_t *rx_msg_p;
  // Other
  uint32_t current_tick;
  static uint8_t initial_soc_measurement_flag = 1;
  bool HLIM_status = false;

  current_tick = HAL_GetTick();

  // Recieve ECU CAN message
  CAN_RecieveMessages(&hcan, rxMessages); // FIXME: convert to interrupt-based instead
  for (int i = 0; i < (NUM_RX_FIFOS * MAX_MESSAGES_PER_FIFO); i++)
  {
    rx_msg_p = &rxMessages[i];

    // operate on message if it's the current status message from the ECU
    if (rx_msg_p->rx_header.StdId == ECU_CURRENT_MESSAGE_ID)
    { // does comparing this way work?
      DOC_COC_active = rx_msg_p->data[0];
      pack_current = rx_msg_p->data[1]; // TODO: double check whether we need to rescale one rx'd (depends on how ECU packages up this value)
      break;                           // remove if we want to rx more than just this message
    }
  }

  if (DOC_COC_active)
  {
    pack->status.bits.fault_over_current = true; // set FLT_DOC_COC bit
  }

  // TODO: isolation sensor check (if it's BMS's responsibilty)

  // get pack measurements
  if (BTM_getVoltages(pack).error != BTM_OK || BTM_TEMP_getTemperatures(pack).error != BTM_OK)
  {
    pack->status.bits.fault_communications = true;
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

  // write pack status code
  ANA_analyzePack(pack);

  // if any fault active, drive FLT, COM, OT GPIOs, turn off balancing, drive fans 100%
  if (PACK_ANY_FAULTS_SET(&(pack->status)))
  {
    CONT_FLT_switch(PACK_ANY_FAULTS_SET(&(pack->status)) ? CONT_ACTIVE : CONT_INACTIVE);
    CONT_COM_switch(pack->status.bits.fault_communications ? CONT_ACTIVE : CONT_INACTIVE);
    CONT_OT_switch(pack->status.bits.fault_over_temperature ? CONT_ACTIVE : CONT_INACTIVE);
    stopBalancing(pack);
    CONT_BAL_switch(pack->status.bits.balancing_active ? CONT_ACTIVE : CONT_INACTIVE);
    CONT_FAN_PWM_set(FAN_FULL);
  }
  else // no fault; balance modules, drive control signals and fans
  {
    BAL_updateBalancing(pack); // write bal settings, send bal commands
    CONT_BAL_switch(pack->status.bits.balancing_active ? CONT_ACTIVE : CONT_INACTIVE);

    // HLIM active if TRIP_HLIM or TRIP_CHARGE_OT are active
    HLIM_status = pack->status.bits.hlim || pack->status.bits.charge_over_temperature_limit;
    CONT_HLIM_switch(HLIM_status ? CONT_ACTIVE : CONT_INACTIVE);
    CONT_LLIM_switch((pack->status.bits.llim) ? CONT_ACTIVE : CONT_INACTIVE);

    // set fans
    if (pack->status.bits.charge_over_temperature_limit) // if TRIP_CHARGE_OT active, drive fans 100%
    {
      fan_PWM = FAN_FULL;
    }
    else // otherwise, fans set proportional to temperature
    {
      max_temp = ANA_findHighestModuleTemp(pack);
      fan_PWM = CONT_fanPwmFromTemp(max_temp);
    }
    CONT_FAN_PWM_set(fan_PWM);
  }

  return;
}

/**
 * @brief Preforms initial system checks
 * Writes COMM bit in status code if checks fail
 */
void startupChecks(Pack_t *pack)
{
  uint8_t test_data[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE] = {{0x55, 0x6E, 0x69, 0x42, 0x43, 0x20}, {0x53, 0x6f, 0x6c, 0x61, 0x72, 0x21}};
  uint8_t test_data_rx[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE] = {0};
  BTM_Status_t comm_status = {BTM_OK, 0};
  bool reg_group_match;

  BTM_writeRegisterGroup(CMD_WRCOMM, test_data);
  comm_status = BTM_readRegisterGroup(CMD_RDCOMM, test_data_rx);
  reg_group_match = doesRegGroupMatch(test_data, test_data_rx);

  // checks for comms error
  // note: a lack of comms is different than the self-tests failing
  if (comm_status.error != BTM_OK)
  {
    pack->status.bits.fault_communications = true;
  }
  if (!reg_group_match)
  {
    pack->status.bits.fault_self_test = true;
  }
}

/**
 * @brief Helper function for initial system checks
 * Returns true for full match, false otherwise
 */
bool doesRegGroupMatch(uint8_t reg_group1[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE],
                       uint8_t reg_group2[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE])
{
  for (int ic_num = 0; ic_num < BTM_NUM_DEVICES; ic_num++)
  {
    for (int i = 0; i < BTM_REG_GROUP_SIZE; i++)
    {
      if (reg_group1[ic_num][i] != reg_group2[ic_num][i])
        return false;
    }
  }
  return true;
}

/**
 * @brief Helper function to stop balancing for all modules
 *
 */
void stopBalancing(Pack_t *pack)
{
  bool discharge_setting[PACK_NUM_BATTERY_MODULES] = {false};
  BTM_BAL_setDischarge(pack, discharge_setting);   // writes balancing commands for all modules
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
