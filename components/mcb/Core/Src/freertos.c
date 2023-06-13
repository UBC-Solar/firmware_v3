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
/* Definitions for UpdateState */
osThreadId_t UpdateStateHandle;
const osThreadAttr_t UpdateState_attributes = {
  .name = "UpdateState",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for SendMotorCommand */
osThreadId_t SendMotorCommandHandle;
const osThreadAttr_t SendMotorCommand_attributes = {
  .name = "SendMotorCommand",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for GetADCValues */
osThreadId_t GetADCValuesHandle;
const osThreadAttr_t GetADCValues_attributes = {
  .name = "GetADCValues",
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

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void updateState(void *argument);
void sendMotorCommand(void *argument);
void getADCValues(void *argument);
void getBatterySOC(void *argument);
void getVelocity(void *argument);

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

  /* creation of UpdateState */
  UpdateStateHandle = osThreadNew(updateState, NULL, &UpdateState_attributes);

  /* creation of SendMotorCommand */
  SendMotorCommandHandle = osThreadNew(sendMotorCommand, NULL, &SendMotorCommand_attributes);

  /* creation of GetADCValues */
  GetADCValuesHandle = osThreadNew(getADCValues, NULL, &GetADCValues_attributes);

  /* creation of GetBatterySOC */
  GetBatterySOCHandle = osThreadNew(getBatterySOC, NULL, &GetBatterySOC_attributes);

  /* creation of GetVelocity */
  GetVelocityHandle = osThreadNew(getVelocity, NULL, &GetVelocity_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Task for debugging
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
	 HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
     osDelay(500);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_updateState */
/**
* @brief Task for setting the state based on the given event_flags.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_updateState */
void updateState(void *argument)
{
	/* USER CODE BEGIN updateState */
	/* Infinite loop */
	for(;;)
	{
		UpdateInputFlags(&input_flags); // Updates certain
		/*
		 *  Conditional statement is intentially organized in a hierarchical structure.
		 *  If there are two valid states based on the given event_flags, the higher one will take priority.
		 *  Ex: If throttle and regen are both pressed, the state will be in regen because it has the higher priority.
		 */
		if (input_flags.park_enabled && (state == IDLE || state == PARK))
			state = PARK;
		else if (input_flags.mech_brake_pressed)
			state = IDLE;
		else if (input_flags.regen_pressed && input_flags.charge_under_threshold)
		  	state = REGEN;
		else if (input_flags.cruise_enabled && input_flags.cruise_accelerate_enabled)
		  	state = CRUISE_ACCELERATE;
		else if (input_flags.cruise_enabled)
		    state = CRUISE;
		else if (input_flags.reverse_enabled && input_flags.velocity_under_threshold && state != PARK)
			state = REVERSE;
		else if (input_flags.throttle_pressed)
		    state = DRIVE;
		else
		  	state = IDLE;
		osDelay(UPDATE_STATE_DELAY);
	}
	/* USER CODE END updateState */
}

/* USER CODE BEGIN Header_sendMotorCommand */
/**
* @brief Task for sending Motor commands via CAN based on the current state
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_sendMotorCommand */
void sendMotorCommand(void *argument)
{
  /* USER CODE BEGIN sendMotorCommand */
	/* Infinite loop */
	for(;;)
	{
		if(state == PARK)
		{
			velocity = 0.0;
			current = 1.0;
		}
		else if(state == REGEN)
	    {
	    	velocity = 0.0;
	    	current = NormalizeADCValue(ADC_regen_val);
	    }
	    else if(state == DRIVE)
	    {
	    	velocity = 100.0;
	    	current = NormalizeADCValue(ADC_throttle_val);
	    }
	    else if(state == REVERSE)
	    {
	    	velocity = -100.0;
	    	current = NormalizeADCValue(ADC_throttle_val);
	    }
	    else if (state == CRUISE)
	    {
	    	velocity = cruise_velocity;
	    	current = 1.0;
	    }
		else if (state == CRUISE_ACCELERATE)
		{
			velocity = 100.0;
			current = NormalizeADCValue(ADC_throttle_val);
		}
	    else
	    {
	    	velocity = 0.0;
	    	current = 0.0;
	    }
		SendCANMotorCommand(velocity, current);
	    osDelay(SEND_MOTOR_COMMAND_DELAY);
	}
  /* USER CODE END sendMotorCommand */
}

/* USER CODE BEGIN Header_getADCValues */
/**
* @brief Gets the throttle and regen ADC values and updates relevent event flags
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_getADCValues */
void getADCValues(void *argument)
{
  /* USER CODE BEGIN getADCValues */
  /* Infinite loop */
  for(;;)
  {
	// Get ADC value for throttle and sets event flags
	ADC_throttle_val = ReadADC(&hadc1);
	input_flags.throttle_pressed = ADC_throttle_val > ADC_DEADZONE;
	input_flags.cruise_accelerate_enabled = NormalizeADCValue(ADC_throttle_val) > CRUISE_CURRENT;

	// Gets ADC value for regen and sets event flags
	ADC_regen_val = ReadADC(&hadc2);
	input_flags.regen_pressed = ADC_regen_val > ADC_DEADZONE;
	if(ADC_regen_val > ADC_DEADZONE)
		input_flags.cruise_enabled = FALSE;

    osDelay(GET_ADC_VALUES_DELAY);
  }
  /* USER CODE END getADCValues */
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
					battery_soc = BATTERY_SOC_FULL;
				else
					battery_soc = battery_msg_data[0];
			}

	  		osDelay(GET_BATTERY_SOC_DELAY);
		}
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
			if (can_rx_header.StdId == 0x503)
			{
				for(int i = 0; i < (sizeof(float)/sizeof(uint8_t)); i++)
				{
					velocity.bytes[i] = CAN_message[i+4]; // Vechicle Velocity is stored in bits 32-63.
				}
				velocity_of_car = velocity.float_value;
			}
		}
		osDelay(GET_VELOCITY_DELAY);
	}
  /* USER CODE END getVelocity */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

