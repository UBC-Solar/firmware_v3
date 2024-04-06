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


GPIO_PinState Reverse;
GPIO_PinState Park;
GPIO_PinState Drive;
GPIO_PinState Regen;


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
/* Definitions for drive_state_machine */
osThreadId_t drive_state_machineHandle;
const osThreadAttr_t drive_state_machine_attributes = {
  .name = "drive_state_machine",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for GetCANMessage */
osThreadId_t GetCANMessageHandle;
const osThreadAttr_t GetCANMessage_attributes = {
  .name = "GetCANMessage",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for generic_100ms */
osThreadId_t generic_100msHandle;
const osThreadAttr_t generic_100ms_attributes = {
  .name = "generic_100ms",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void task_drive_state_machine(void *argument);
void getCANMessage(void *argument);
void task_generic_100ms(void *argument);

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

  /* creation of drive_state_machine */
  drive_state_machineHandle = osThreadNew(task_drive_state_machine, NULL, &drive_state_machine_attributes);

  /* creation of GetCANMessage */
  GetCANMessageHandle = osThreadNew(getCANMessage, NULL, &GetCANMessage_attributes);

  /* creation of generic_100ms */
  generic_100msHandle = osThreadNew(task_generic_100ms, NULL, &generic_100ms_attributes);

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
	 HAL_WWDG_Refresh(&hwwdg);
	 osDelay(100);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_task_drive_state_machine */
/**
* @brief Function implementing the drive_state_machine thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_task_drive_state_machine */
void task_drive_state_machine(void *argument)
{
  /* USER CODE BEGIN task_drive_state_machine */
  /* Infinite loop */
  for(;;)
  {
    taskENTER_CRITICAL();

    drive_state_machine_handler();

    taskEXIT_CRITICAL();
	  osDelay(DELAY_MCB_STATE_MACHINE);
  }
  /* USER CODE END task_drive_state_machine */
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
	TaskGetCANMessage();
  /* USER CODE END getCANMessage */
}

/* USER CODE BEGIN Header_task_generic_100ms */
/**
* @brief Function implementing the generic_100ms thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_task_generic_100ms */
void task_generic_100ms(void *argument)
{
  /* USER CODE BEGIN task_generic_100ms */
  /* Infinite loop */
  for(;;)
  {
    // Sends MCB drive state
    SendCANDIDDriveState();

    osDelay(100);
  }
  /* USER CODE END task_generic_100ms */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

