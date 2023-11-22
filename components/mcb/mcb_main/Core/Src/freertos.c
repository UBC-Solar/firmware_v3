/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "mcb.h"
#include "can.h"
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
/* USER CODE BEGIN Variables */
uint16_t ADC_throttle_val;
uint16_t ADC_regen_val;


/*
 *  These are global so I can view their value in the Live Expressions tab, will move to task later.
 */
float current;
float velocity;

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for GetBatterySOC */
osThreadId_t GetBatterySOCHandle;
const osThreadAttr_t GetBatterySOC_attributes = {
  .name = "GetBatterySOC",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for GetVelocity */
osThreadId_t GetVelocityHandle;
const osThreadAttr_t GetVelocity_attributes = {
  .name = "GetVelocity",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for GetBatteryTemp */
osThreadId_t GetBatteryTempHandle;
const osThreadAttr_t GetBatteryTemp_attributes = {
  .name = "GetBatteryTemp",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for MCBStateMachine */
osThreadId_t MCBStateMachineHandle;
const osThreadAttr_t MCBStateMachine_attributes = {
  .name = "MCBStateMachine",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for GetCANMessage */
osThreadId_t GetCANMessageHandle;
const osThreadAttr_t GetCANMessage_attributes = {
  .name = "GetCANMessage",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void getBatterySOC(void *argument);
void getVelocity(void *argument);
void getBatteryTemp(void *argument);
void mcbStateMachine(void *argument);
void getCANMessage(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of GetBatterySOC */
  GetBatterySOCHandle = osThreadNew(getBatterySOC, NULL, &GetBatterySOC_attributes);

  /* creation of GetVelocity */
  GetVelocityHandle = osThreadNew(getVelocity, NULL, &GetVelocity_attributes);

  /* creation of GetBatteryTemp */
  GetBatteryTempHandle = osThreadNew(getBatteryTemp, NULL, &GetBatteryTemp_attributes);

  /* creation of MCBStateMachine */
  MCBStateMachineHandle = osThreadNew(mcbStateMachine, NULL, &MCBStateMachine_attributes);

  /* creation of GetCANMessage */
  GetCANMessageHandle = osThreadNew(getCANMessage, NULL, &GetCANMessage_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
	 HAL_GPIO_TogglePin(LED_OUT1_GPIO_Port, LED_OUT1_Pin);
	 HAL_GPIO_TogglePin(LED_OUT2_GPIO_Port, LED_OUT2_Pin);
     osDelay(500);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_getBatterySOC */
/**
* @brief Gets battery state of charge from incoming CAN messages.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_getBatterySOC */
void getBatterySOC(void *argument)
{
  /* USER CODE BEGIN getBatterySOC */
	uint8_t battery_msg_data[8];
	/* Infinite loop */
	for(;;)
	{
		if (HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_RX_FIFO0))
		{
			// there are multiple CAN IDs being passed through the filter, check if the message is the SOC
			HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &can_rx_header, battery_msg_data);
			if (can_rx_header.StdId == 0x626)
			{
				// if the battery SOC is out of range, assume it is at 100% as a safety measure
				if (battery_msg_data[0] < BATTERY_SOC_EMPTY || battery_msg_data[0] > BATTERY_SOC_FULL)
					gBatterySOC = BATTERY_SOC_FULL;
				else
					gBatterySOC = battery_msg_data[0];
			}

	  		osDelay(GET_BATTERY_SOC_DELAY);
		}
		osDelay(GET_BATTERY_SOC_DELAY);
	}
  /* USER CODE END getBatterySOC */
}

/* USER CODE BEGIN Header_getVelocity */
/**
* @brief Gets the velocity of the car from incoming CAN messages
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_getVelocity */
void getVelocity(void *argument)
{
  /* USER CODE BEGIN getVelocity */
	uint8_t CAN_message[8];
	FloatBytes velocity;
	/* Infinite loop */
	for(;;)
	{
		if (HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_RX_FIFO0))
		{
			// there are multiple CAN IDs being passed through the filter, check if the message is the SOC
			HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &can_rx_header, CAN_message);
			if (can_rx_header.StdId == GET_VELOCITY_HEADER)
			{
				for(int i = 0; i < (sizeof(float)/sizeof(uint8_t)); i++)
				{
					velocity.bytes[i] = CAN_message[i+4]; // Vehicle Velocity is stored in bits 32-63.
				}
				gVelocityOfCar = velocity.float_value;
			}
		}
		osDelay(GET_VELOCITY_DELAY);

	}
  /* USER CODE END getVelocity */
}

/* USER CODE BEGIN Header_getBatteryTemp */
/**
* @brief Function implementing the GetBatteryTemp thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_getBatteryTemp */
void getBatteryTemp(void *argument)
{
  /* USER CODE BEGIN getBatteryTemp */
	uint8_t CAN_message[8];
	IntBytes battery_message;
	/* Infinite loop */
	for(;;)
	{
		if (HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_RX_FIFO0))
		{
			// there are multiple CAN IDs being passed through the filter, check if the message is the battery temp
			HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &can_rx_header, CAN_message);
			if (can_rx_header.StdId == BATTERY_MESSAGE_HEADER)
			{
				for(int i = 0; i < (sizeof(float)/sizeof(uint8_t)); i++)
				{
					battery_message.bytes[i] = CAN_message[i]; // regen_disable bit is stored in bit 17
				}
				input_flags.regen_disable = isBitSet(battery_message.int_value, 17);
			}
		}
		osDelay(GET_VELOCITY_DELAY);
	}
  /* USER CODE END getBatteryTemp */
}

/* USER CODE BEGIN Header_mcbStateMachine */
/**
* @brief Function implementing the MCBStateMachine thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_mcbStateMachine */
void mcbStateMachine(void *argument)
{
  /* USER CODE BEGIN mcbStateMachine */
	TaskMCBStateMachine();
  /* USER CODE END mcbStateMachine */
}

/* USER CODE BEGIN Header_getCANMessage */
/**
* @brief Function implementing the GetCANMessage thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_getCANMessage */
void getCANMessage(void *argument)
{
  /* USER CODE BEGIN getCANMessage */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END getCANMessage */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

