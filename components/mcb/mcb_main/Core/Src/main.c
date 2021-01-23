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

#include "main.h"
#include "encoder.h"
#include "timer.h"

CAN_HandleTypeDef hcan;
TIM_HandleTypeDef htim3;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void initializeGPIO(void);
void MX_CAN_Init(void);

#define DRIVER_CONTROLS_BASE_ADDRESS 0x400
#define DATA_FRAME_LEN 8

// motor needs to be sent a CAN message every 250ms so timeout value is a conservative 200ms
#define MOTOR_TIMEOUT_MS 200

union {
	float float_value;
	uint8_t bytes[4];
} current;

union {
	float float_value;
	uint8_t bytes[4];
} velocity;

int main(void)
{
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  SystemClock_Config();

  /* Initialize all configured peripherals */
  initializeGPIO();
  MX_CAN_Init();
  MX_TIM3_Init(MOTOR_TIMEOUT_MS);

  volatile uint16_t encoder_reading;
  uint16_t old_encoder_reading = 0x0000;

  HAL_CAN_Start(&hcan);

  // starts TIM3 with interrupts being sent at every update event
  HAL_TIM_Base_Start_IT(&htim3);

  // set the priority of the TIM3 interrupt
  HAL_NVIC_SetPriority(TIM3_IRQn, 1, 0);

  // enabling interrupts
  HAL_NVIC_EnableIRQ(TIM3_IRQn);

  /* Create CAN header struct for drive command */
  // TODO: might have to initialize all the values inside the struct
  CAN_TxHeaderTypeDef drive_command_header;
  drive_command_header.StdId = DRIVER_CONTROLS_BASE_ADDRESS + 1;
  drive_command_header.IDE = CAN_ID_STD;

  /* CAN Mailbox variable (stores the mailbox that the CAN message was sent to) */
  uint32_t can_mailbox;

  /* Initialize velocity and motor current values */
  velocity.float_value = 100.0;
  current.float_value = 0.0;

  // convert encoder reading to interrupt
  while (1)
  {
    encoder_reading = EncoderRead();

    // checks if the RVRS_EN pin is enabled
    if (HAL_GPIO_ReadPin(RVRS_EN_GPIO_Port, RVRS_EN_Pin)) {
    	velocity.float_value = -100.0;
    } else {
    	velocity.float_value = 100.0;
    }

    // if the encoder reading changes, send the correct CAN message, and restart the timer
    if (old_encoder_reading != encoder_reading) {

    	// linear scaling for encoder
    	current.float_value = ((float) encoder_reading / (float) PEDAL_MAX);

    	/* send a new drive command */

    	uint8_t data_send[DATA_FRAME_LEN];

    	// inserting the velocity and current values into the data_send array
    	// TODO: make this a function?
    	for (int i = 0; i < DATA_FRAME_LEN / 2; i++) {
    		data_send[i] = velocity.bytes[i];
    		data_send[i + 4] = current.bytes[i];
    	}

    	// adds a CAN message to the transmit mailbox
    	// TODO: add some sort of error handling if the CAN message isn't sent properly
    	HAL_CAN_AddTxMessage(&hcan, &drive_command_header, data_send, &can_mailbox);

    	// restart timer

    }

    old_encoder_reading = encoder_reading;

  }
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
}

/**
  * @brief CAN Initialization Function
  * @param None
  * @retval None
  */
void MX_CAN_Init(void)
{
  hcan.Instance = CAN1;
  hcan.Init.Prescaler = 16;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_1TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_1TQ;
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
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
void initializeGPIO(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(CRUISE_STAT_GPIO_Port, CRUISE_STAT_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : REGEN_VAL_Pin REGEN_EN_Pin BRK_IN_Pin RVRS_EN_Pin
                           CRUISE_EN_Pin CRUISE_DIS_Pin CRUISE_UP_Pin CRUISE_DOWN_Pin
                           ENC_AS_Pin ENC_BS_Pin SWDIO_Pin SWCLK_Pin
                           ENC_ZS_Pin */
  GPIO_InitStruct.Pin = REGEN_VAL_Pin|REGEN_EN_Pin|BRK_IN_Pin|RVRS_EN_Pin
                          |CRUISE_EN_Pin|CRUISE_DIS_Pin|CRUISE_UP_Pin|CRUISE_DOWN_Pin
                          |ENC_AS_Pin|ENC_BS_Pin|SWDIO_Pin|SWCLK_Pin
                          |ENC_ZS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : CRUISE_STAT_Pin */
  GPIO_InitStruct.Pin = CRUISE_STAT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(CRUISE_STAT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : ENC_YS_Pin */
  GPIO_InitStruct.Pin = ENC_YS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ENC_YS_GPIO_Port, &GPIO_InitStruct);

}

/**
  * @brief  This function is executed in case of error occurrence.
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
