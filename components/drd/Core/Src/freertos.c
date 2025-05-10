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
#include "drive_state.h"
#include "iwdg.h"
#include "diagnostic.h"
#include "cyclic_data_handler.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define DEFAULT_TASK_DELAY 			  100
#define TIME_SINCE_STARTUP_TASK_DELAY 1000
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
uint32_t defaultTaskBuffer[ 128 ];
osStaticThreadDef_t defaultTaskControlBlock;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .cb_mem = &defaultTaskControlBlock,
  .cb_size = sizeof(defaultTaskControlBlock),
  .stack_mem = &defaultTaskBuffer[0],
  .stack_size = sizeof(defaultTaskBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for ExtLightsTask */
osThreadId_t ExtLightsTaskHandle;
uint32_t ExternalLights_Buffer[ 256 ];
osStaticThreadDef_t ExternalLights_ControlBlock;
const osThreadAttr_t ExtLightsTask_attributes = {
  .name = "ExtLightsTask",
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
/* Definitions for DriveStateTask */
osThreadId_t DriveStateTaskHandle;
uint32_t DriveStateTaskBuffer[ 256 ];
osStaticThreadDef_t DriveStateTaskControlBlock;
const osThreadAttr_t DriveStateTask_attributes = {
  .name = "DriveStateTask",
  .cb_mem = &DriveStateTaskControlBlock,
  .cb_size = sizeof(DriveStateTaskControlBlock),
  .stack_mem = &DriveStateTaskBuffer[0],
  .stack_size = sizeof(DriveStateTaskBuffer),
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for TimeSinceStartu */
osThreadId_t TimeSinceStartuHandle;
uint32_t TimeSinceStartuBuffer[ 128 ];
osStaticThreadDef_t TimeSinceStartuControlBlock;
const osThreadAttr_t TimeSinceStartu_attributes = {
  .name = "TimeSinceStartu",
  .cb_mem = &TimeSinceStartuControlBlock,
  .cb_size = sizeof(TimeSinceStartuControlBlock),
  .stack_mem = &TimeSinceStartuBuffer[0],
  .stack_size = sizeof(TimeSinceStartuBuffer),
  .priority = (osPriority_t) osPriorityLow,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void ExternalLights_task(void *argument);
void LCDUpdatetask(void *argument);
void DriveState_task(void *argument);
void TimeSinceStartup_task(void *argument);

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

  /* creation of ExtLightsTask */
  ExtLightsTaskHandle = osThreadNew(ExternalLights_task, NULL, &ExtLightsTask_attributes);

  /* creation of LCDUpdateTask */
  LCDUpdateTaskHandle = osThreadNew(LCDUpdatetask, NULL, &LCDUpdateTask_attributes);

  /* creation of DriveStateTask */
  DriveStateTaskHandle = osThreadNew(DriveState_task, NULL, &DriveStateTask_attributes);

  /* creation of TimeSinceStartu */
  TimeSinceStartuHandle = osThreadNew(TimeSinceStartup_task, NULL, &TimeSinceStartu_attributes);

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
  DRD_diagnostics_transmit(&g_diagnostics, false);
  /* Infinite loop */
  for(;;)
  {
	  Motor_Controller_query_data();
	  IWDG_Refresh(&hiwdg);
    osDelay(DEFAULT_TASK_DELAY);
    DRD_diagnostics_transmit(&g_diagnostics, false); //transmit diagnostics every 100ms
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

  g_lcd_data.speed_units = KPH;
  
  for(;;)
  {
    g_lcd_data.speed = get_cyclic_speed();
    g_lcd_data.drive_state = get_cyclic_drive_state();
    g_lcd_data.drive_mode = (volatile uint8_t) g_input_flags.eco_mode_on;

    LCD_display_power_bar((volatile float*) g_lcd_data.pack_current, (volatile float*) g_lcd_data.pack_voltage);
    LCD_display_speed(g_lcd_data.speed, g_lcd_data.speed_units);
    LCD_display_drive_state(g_lcd_data.drive_state);
    LCD_display_SOC((volatile uint32_t*) g_lcd_data.soc);
    LCD_display_drive_mode(g_lcd_data.drive_mode);

    osDelay(LCD_UPDATE_DELAY);
  }
  /* USER CODE END LCDUpdatetask */
}

/* USER CODE BEGIN Header_DriveState_task */
/**
* @brief Function implementing the DriveStateTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_DriveState_task */
void DriveState_task(void *argument)
{
  /* USER CODE BEGIN DriveState_task */
	g_input_flags.velocity_under_threshold = true;

  /* Infinite loop */
  for(;;)
  {
    osDelay(DRIVE_STATE_MACHINE_DELAY);
    Drive_State_Machine_handler();
  }
  /* USER CODE END DriveState_task */
}

/* USER CODE BEGIN Header_TimeSinceStartup_task */
/**
* @brief Function implementing the TimeSinceStartu thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_TimeSinceStartup_task */
void TimeSinceStartup_task(void *argument)
{
  /* USER CODE BEGIN TimeSinceStartup_task */
  /* Infinite loop */
  for(;;)
  {
    g_time_since_bootup++;
    osDelay(TIME_SINCE_STARTUP_TASK_DELAY);
    DRD_time_since_bootup();
  }
  /* USER CODE END TimeSinceStartup_task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

