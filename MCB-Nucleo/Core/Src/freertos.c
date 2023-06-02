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

// TODO Add gpio inputs (park, reverse, mech_brake) to event_flags?
// - Adding park reverse and mech_brake to event_flags will in theory lower performance but greatly improve code readablity
// TODO Confirm velocity readings will be signed.
// TODO Confirm gpio input pins can use interrupts
// TODO Next page button

/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usart.h"
#include "adc.h"
#include "can.h"
#include "MCB.h"
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

union FloatBytes velocity_of_car; // Current velocity of the car will be stored here.

struct InputFlags event_flags; // Event flags for deciding what state to be in.

float cruise_velocity;

uint8_t battery_soc; // Stores the charge of the battery, updated in a task.

float current; // These are global so I can view their value in the Live Expressions tab, will move to task later.
float velocity;

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
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
/* Definitions for updateFlags */
osThreadId_t updateFlagsHandle;
const osThreadAttr_t updateFlags_attributes = {
  .name = "updateFlags",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for motorStateMachi */
osThreadId_t motorStateMachiHandle;
const osThreadAttr_t motorStateMachi_attributes = {
  .name = "motorStateMachi",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for getCANVelocity */
osThreadId_t getCANVelocityHandle;
const osThreadAttr_t getCANVelocity_attributes = {
  .name = "getCANVelocity",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for getCANBatterySO */
osThreadId_t getCANBatterySOHandle;
const osThreadAttr_t getCANBatterySO_attributes = {
  .name = "getCANBatterySO",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for updateState */
osThreadId_t updateStateHandle;
const osThreadAttr_t updateState_attributes = {
  .name = "updateState",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void getADCValues(void *argument);
void startUpdateFlags(void *argument);
void startMotorStateMachine(void *argument);
void startGetCANVelocity(void *argument);
void StartSetCANBatterySO(void *argument);
void StartUpdateState(void *argument);

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

  /* creation of GetADCValues */
  GetADCValuesHandle = osThreadNew(getADCValues, NULL, &GetADCValues_attributes);

  /* creation of updateFlags */
  updateFlagsHandle = osThreadNew(startUpdateFlags, NULL, &updateFlags_attributes);

  /* creation of motorStateMachi */
  motorStateMachiHandle = osThreadNew(startMotorStateMachine, NULL, &motorStateMachi_attributes);

  /* creation of getCANVelocity */
  getCANVelocityHandle = osThreadNew(startGetCANVelocity, NULL, &getCANVelocity_attributes);

  /* creation of getCANBatterySO */
  getCANBatterySOHandle = osThreadNew(StartSetCANBatterySO, NULL, &getCANBatterySO_attributes);

  /* creation of updateState */
  updateStateHandle = osThreadNew(StartUpdateState, NULL, &updateState_attributes);

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
	 HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
     osDelay(500);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_getADCValues */
/**
* @brief Function implementing the GetADCValues thread.
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
	// Get ADC value for throttle and sets event flag
	ADC_throttle_val = ReadADC(&hadc1);
	event_flags.throttle_pressed = ADC_throttle_val > ADC_DEADZONE;

	// Gets ADC value for regen and sets event flags
	ADC_regen_val = ReadADC(&hadc2);
	event_flags.regen_pressed = ADC_regen_val > ADC_DEADZONE;
	if(ADC_regen_val > ADC_DEADZONE)
		event_flags.cruise_enabled = FALSE;

    osDelay(ADC_POLL_DELAY);
  }
  /* USER CODE END getADCValues */
}

/* USER CODE BEGIN Header_startUpdateFlags */
/**
* @brief Function implementing the updateFlags thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_startUpdateFlags */
void startUpdateFlags(void *argument)
{
  /* USER CODE BEGIN startUpdateFlags */
  /* Infinite loop */
  for(;;)
  {

	  event_flags.mech_brake_pressed = HAL_GPIO_ReadPin(MECH_BRAKE_GPIO_Port, MECH_BRAKE_Pin);

	  event_flags.park_enabled = HAL_GPIO_ReadPin(SWITCH_PARK_GPIO_Port, SWITCH_PARK_Pin);

	  event_flags.reverse_enabled = HAL_GPIO_ReadPin(SWITCH_REVERSE_GPIO_Port, SWITCH_REVERSE_Pin);

	  event_flags.velocity_under_threshold = (velocity_of_car.float_value < MIN_REVERSE_VELOCITY);

	  event_flags.charge_under_threshold = battery_soc < BATTERY_SOC_THRESHHOLD;

	  osDelay(EVENT_FLAG_UPDATE_DELAY);
  }
  /* USER CODE END startUpdateFlags */
}

/* USER CODE BEGIN Header_startMotorStateMachine */
/**
* @brief Function implementing the motorStateMachi thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_startMotorStateMachine */
void startMotorStateMachine(void *argument)
{
  /* USER CODE BEGIN startMotorStateMachine */

  /* Infinite loop */
  for(;;)
  {
	if(state == PARK)
	{
		velocity = 0.0;
		current = 0.0;
	}
	else if(state == REGEN)
    {
    	velocity = 0.0;
    	current = NormalizeValue(ADC_regen_val);
    }
    else if(state == DRIVE)
    {

    	velocity = 100.0;
    	current = NormalizeValue(ADC_throttle_val);
    }
    else if(state == REVERSE)
    {
    	velocity = -100.0;
    	current = NormalizeValue(ADC_throttle_val);
    }
    else if (state == CRUISE)
    {
    	velocity = cruise_velocity;
    	current = 1.0;
    }
    else
    {
    	velocity = 0.0;
    	current = 0.0;
    }

	SendCANMotorCommand(velocity, current);
    osDelay(DELAY_MOTOR_STATE_MACHINE);
  }
  /* USER CODE END startMotorStateMachine */
}

/* USER CODE BEGIN Header_startGetCANVelocity */
/**
* @brief Function implementing the getCANVelocity thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_startGetCANVelocity */
void startGetCANVelocity(void *argument)
{
  /* USER CODE BEGIN startGetCANVelocity */
	uint8_t CAN_message[8];
	/* Infinite loop */
	for(;;)
	{
		if (HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_RX_FIFO0))
		{
			// there are multiple CAN IDs being passed through the filter, check if the message is the SOC
			HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &can_rx_header, CAN_message);
			if (can_rx_header.StdId == 0x503)
			{
				for(int i= 0; i < 4; i++)
				{
					velocity_of_car.bytes[i] = CAN_message[i+4]; // Vechicle Velocity is stored in bits 32-63.
				}
			}
		}
		osDelay(GET_CAN_VELOCITY_DELAY);
	}
  /* USER CODE END startGetCANVelocity */
}

/* USER CODE BEGIN Header_StartSetCANBatterySO */
/**
* @brief Function implementing the getCANBatterySO thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartSetCANBatterySO */
void StartSetCANBatterySO(void *argument)
{
  /* USER CODE BEGIN StartSetCANBatterySO */
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
				if (battery_msg_data[0] < 0 || battery_msg_data[0] > 100)
					battery_soc = 100;
				else
					battery_soc = battery_msg_data[0];
			}

	  		osDelay(READ_BATTERY_SOC_DELAY);
		}
	}
  /* USER CODE END StartSetCANBatterySO */
}

/* USER CODE BEGIN Header_StartUpdateState */
/**
* @brief Function implementing the updateState thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartUpdateState */
void StartUpdateState(void *argument)
{
  /* USER CODE BEGIN StartUpdateState */
  /* Infinite loop */
  for(;;)
  {
	  if (event_flags.park_enabled && (state == IDLE || state == PARK || state == REVERSE))
		  state = PARK;
	  else if (event_flags.mech_brake_pressed)
		  state = IDLE;
	  else if (event_flags.regen_pressed && event_flags.charge_under_threshold)
	  	  state = REGEN;
	  else if (event_flags.cruise_enabled)
	      state = CRUISE;
	  else if (event_flags.reverse_enabled && event_flags.velocity_under_threshold)
		  state = REVERSE;
	  else if (event_flags.throttle_pressed)
	      state = DRIVE;
	  else
	  	  state = IDLE;

	  osDelay(EVENT_FLAG_UPDATE_DELAY);
  }
  /* USER CODE END StartUpdateState */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == BTN_CRUISE_TOGGLE_Pin)
	{

		if(state == DRIVE || state == CRUISE)
		{
			event_flags.cruise_enabled = !event_flags.cruise_enabled;
			cruise_velocity = velocity_of_car.float_value;
		}
	}
	else if(GPIO_Pin == BTN_CRUISE_UP_Pin)
	{
		if(state == CRUISE)
			if(cruise_velocity + CRUISE_INCREMENT_VAL < CRUISE_MAX)
				cruise_velocity += CRUISE_INCREMENT_VAL;
			else
				cruise_velocity = CRUISE_MAX;
	}
	else if(GPIO_Pin == BTN_CRUISE_DOWN_Pin)
	{
		if(state == CRUISE)
			if(cruise_velocity - CRUISE_INCREMENT_VAL > CRUISE_MIN)
				cruise_velocity -= CRUISE_INCREMENT_VAL;
			else
				cruise_velocity = CRUISE_MIN;
	}
	else if (GPIO_Pin == MECH_BRAKE_Pin)
	{
		SendCANMotorCommand(0, 0);
		event_flags.cruise_enabled = FALSE;
	}
}
/* USER CODE END Application */

