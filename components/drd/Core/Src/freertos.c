/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "can.h"
#include "external_lights.h"
#include "lcd.h"
#include "spi.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
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

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for ExternalLights_ */
osThreadId_t ExternalLights_Handle;
uint32_t ExternalLights_Buffer[ 256 ];
osStaticThreadDef_t ExternalLights_ControlBlock;
const osThreadAttr_t ExternalLights__attributes = {
  .name = "ExternalLights_",
  .cb_mem = &ExternalLights_ControlBlock,
  .cb_size = sizeof(ExternalLights_ControlBlock),
  .stack_mem = &ExternalLights_Buffer[0],
  .stack_size = sizeof(ExternalLights_Buffer),
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for LCDUpdateTask */
osThreadId_t LCDUpdateTaskHandle;
uint32_t LCDUpdateTaskBuffer[ 256 ];
osStaticThreadDef_t LCDUpdateTaskControlBlock;
const osThreadAttr_t LCDUpdateTask_attributes = {
  .name = "LCDUpdateTask",
  .cb_mem = &LCDUpdateTaskControlBlock,
  .cb_size = sizeof(LCDUpdateTaskControlBlock),
  .stack_mem = &LCDUpdateTaskBuffer[0],
  .stack_size = sizeof(LCDUpdateTaskBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void ExternalLights_task(void *argument);
void LCDUpdatetask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
	CAN_tasks_init();
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

  /* creation of ExternalLights_ */
  ExternalLights_Handle = osThreadNew(ExternalLights_task, NULL, &ExternalLights__attributes);

  /* creation of LCDUpdateTask */
  LCDUpdateTaskHandle = osThreadNew(LCDUpdatetask, NULL, &LCDUpdateTask_attributes);

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
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_ExternalLights_task */
/**
* @brief Function implementing the ExternalLights_ thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_ExternalLights_task */
void ExternalLights_task(void *argument)
{
  /* USER CODE BEGIN ExternalLights_task */
  /* Infinite loop */
  for(;;)
  {
	  ExternalLights_state_machine();

	  osDelay(LIGHTS_STATE_MACHINE_DELAY);

  }
  /* USER CODE END ExternalLights_task */
}

/* USER CODE BEGIN Header_LCDUpdatetask */
/**
* @brief Function implementing the LCDUpdateTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_LCDUpdatetask */
void LCDUpdatetask(void *argument)
{
  /* USER CODE BEGIN LCDUpdatetask */
  /* Infinite loop */
  LCD_init(&hspi1);
  int v1 = 0;
  float v2 = -20;
  for(;;)
  {
    LCD_display_power_bar(v2, 130.0f);
    LCD_display_speed(v1 % 100, 1);
    LCD_display_drive_state(v1 % 5);
    LCD_display_SOC(v1 % 101);  

    ++v1;
    v2 += 0.4;

    osDelay(200);
  }
  /* USER CODE END LCDUpdatetask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

