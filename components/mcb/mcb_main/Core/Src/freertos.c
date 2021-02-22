/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "encoder.h"
#include "can.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define ENCODER_QUEUE_MSG_CNT 5
#define ENCODER_QUEUE_MSG_SIZE 2	/* 2 bytes (uint16_t) */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

osThreadId_t readEncoderTaskHandle;
osThreadId_t sendMotorCommandTaskHandle;
osThreadId_t readRegenValueTaskHandle;
osThreadId_t updateEventFlagsTaskHandle;

osTimerId_t encoderTimerHandle;

osMessageQueueId_t encoderQueueHandle;

// <----- Attribute structs ----->

const osThreadAttr_t readEncoderTask_attributes = {
    .name = "readEncoder",
    .priority = (osPriority_t) osPriorityHigh,
    .stack_size = 128 * 4
};

const osThreadAttr_t sendMotorCommand_attributes = {
    .name = "sendMotorCommand",
    .priority = (osPriority_t) osPriorityHigh,
    .stack_size = 128 * 4
};

const osTimerAttr_t encoderTimer_attributes = {
    .name = "encoderTimer",
    .attr_bits = 0,
    .cb_mem = NULL,
    .cb_size = 0
};

const osThreadAttr_t readRegenValueTask_attributes = {
    .name = "readRegenValue",
    .priority = (osPriority_t) osPriorityHigh,
    .stack_size = 128 * 4
};

const osThreadAttr_t updateEventFlagsTask_attributes = {
    .name = "updateEventFlags",
    .priority = (osPriority_t) osPriorityHigh,
    .stack_size = 128 * 4
};

const osMessageQueueAttr_t encoderQueue_attributes = {
	.name = "encoderQueue"
};


/* USER CODE END Variables */

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

// <----- Thread prototypes ----->

void readEncoderTask(void *argument);
void sendMotorCommandTask(void *argument);
void updateEventFlagsTask(void *argument);

// <----- Timer callback prototypes ----->

static void encoderTimerCallback(void *argument);

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

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

  encoderTimerHandle = osTimerNew(encoderTimerCallback, osTimerPeriodic, NULL, &encoderTimer_attributes);

  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */

  encoderQueueHandle = osMessageQueueNew(ENCODER_QUEUE_MSG_CNT, ENCODER_QUEUE_MSG_SIZE, &encoderQueue_attributes);

  /* USER CODE END RTOS_QUEUES */

  /* USER CODE BEGIN RTOS_THREADS */

  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  readEncoderTaskHandle = osThreadNew(readEncoderTask, NULL, &readEncoderTask_attributes);
  sendMotorCommandTaskHandle = osThreadNew(sendMotorCommandTask, NULL, &sendMotorCommand_attributes);
  updateEventFlagsTaskHandle = osThreadNew(updateEventFlagsTask, NULL, &updateEventFlagsTask_attributes);

  //  readRegenValueTaskHandle = osThreadNew(readRegenValueTask, NULL, &readRegenValueTask_attributes);

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

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

// reads the encoder and places the value in the encoder queue every 1ms
void readEncoderTask(void *argument)
{
    uint16_t encoder_reading;
    uint16_t old_encoder_reading = 0x0000;

    EncoderInit();

    while (1)
    {
      // waits for flags to be set by the timer callback method
      osThreadFlagsWait(0x0001U, osFlagsWaitAny, osWaitForever);

      encoder_reading = EncoderRead();

      // if the encoder value has changed, then put it in the encoder value queue
      if (encoder_reading != old_encoder_reading) {
    	  osMessageQueuePut(encoderQueueHandle, &encoder_reading, 0U, 0U);
      }

      old_encoder_reading = encoder_reading;

      osThreadYield();
    }
}

// sending a CAN message thread
void sendMotorCommandTask(void *argument) {
	uint8_t data_send[DATA_FRAME_LEN];
	osStatus_t status;
	uint16_t encoder_value;

	// velocity is set to unattainable value for motor torque-control mode
	velocity.float_value = 100.0;

	for(;;) {
		// blocks thread waiting for encoder value to be added to queue
//		event = osMessageGet(encoder_queue_id, osWaitForever);
		status = osMessageQueueGet(encoderQueueHandle, &encoder_value, NULL, 0U);

		if (status == osOK) {
			// current is linearly scaled to pedal position
			current.float_value = (float) encoder_value / (PEDAL_MAX - PEDAL_MIN);
		} else {
			// FIXME: change this to better deal with a failure to retrieve value from queue
			osThreadYield();
		}

		// writing data into data_send array which will be sent as a CAN message
		for (int i = 0; i < DATA_FRAME_LEN / 2; i++) {
		  data_send[i] = velocity.bytes[i];
		  data_send[i + 4] = current.bytes[i];
		}

		HAL_CAN_AddTxMessage(&hcan, &drive_command_header, data_send, &can_mailbox);

		osThreadYield();
	}
}

// updates specific flags to decide which thread will send a CAN message
void updateEventFlagsTask(void *argument) {
	while (1) {
		// waits for the EXTI ISRs to set the thread flag
		osThreadFlagsWait(0x0001U, osFlagsWaitAny, osWaitForever);

		// value has been written to the struct, convert this into a number
		uint32_t input_flags = (event_flags.reverse_enable << 2) | (event_flags.brake_in << 1) | (event_flags.regen_enable);

		// now use event flags to signal the above number
		// TODO: finish this

	}
}

// encoder timer callback function
static void encoderTimerCallback(void *argument) {
	// 0x0001 => it is time to read the encoder
	osThreadFlagsSet(readEncoderTaskHandle, 0x0001U);
}

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
