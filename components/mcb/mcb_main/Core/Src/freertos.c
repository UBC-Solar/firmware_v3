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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "attributes.h"
#include "can.h"
#include "encoder.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define ENCODER_QUEUE_MSG_CNT 5
#define ENCODER_QUEUE_MSG_SIZE 2    /* 2 bytes (uint16_t) */

#define CAN_FIFO0 0
#define CAN_FIFO1 1

#define INIT_SEMAPHORE_VALUE 0

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

osThreadId_t readEncoderTaskHandle;
osThreadId_t sendMotorCommandTaskHandle;
osThreadId_t sendRegenCommandTaskHandle;
osThreadId_t updateEventFlagsTaskHandle;

osTimerId_t encoderTimerHandle;

osMessageQueueId_t encoderQueueHandle;

osEventFlagsId_t inputEventFlagsHandle;

osSemaphoreId_t eventFlagsSemaphoreHandle;

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

void updateEventFlagsTask(void *argument);

void sendMotorCommandTask(void *argument);
void sendRegenCommandTask(void *argument);

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

    // <----- Timer object handles ----->

    encoderTimerHandle = osTimerNew(encoderTimerCallback, osTimerPeriodic, NULL, &encoderTimer_attributes);

    // <----- Queue object handles ----->

    encoderQueueHandle = osMessageQueueNew(ENCODER_QUEUE_MSG_CNT, ENCODER_QUEUE_MSG_SIZE, &encoderQueue_attributes);

    // <----- Thread object handles ----->

    defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

    readEncoderTaskHandle = osThreadNew(readEncoderTask, NULL, &readEncoderTask_attributes);
    
    sendMotorCommandTaskHandle = osThreadNew(sendMotorCommandTask, NULL, &sendMotorCommandTask_attributes);
    sendRegenCommandTaskHandle = osThreadNew(sendRegenCommandTask, NULL, &sendRegenCommandTask_attributes);

    updateEventFlagsTaskHandle = osThreadNew(updateEventFlagsTask, NULL, &updateEventFlagsTask_attributes);

    // <----- Event flag object handles ----->

    inputEventFlagsHandle = osEventFlagsNew(NULL);

    // <----- Semaphore object handles ----->

    eventFlagsSemaphoreHandle = osSemaphoreNew(1, INIT_SEMAPHORE_VALUE, NULL);

    // FIXME: this needs to be moved to its own thread or the timer won't start
    osTimerStart(encoderTimerHandle, ENCODER_TIMER_TICKS);

    /* USER CODE END Init */

    /* Create the thread(s) */
    /* creation of defaultTask */
    defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

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
    for (;;) {
        osDelay(1);
    }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

// reads the encoder and places the value in the encoder queue every 1ms
void readEncoderTask(void *argument) {
    uint16_t encoder_reading;
    uint16_t old_encoder_reading = 0x0000U;

    EncoderInit();

    while (1) {
        // waits for flags to be set by the timer callback method
        osThreadFlagsWait(0x0001U, osFlagsWaitAny, osWaitForever);

        encoder_reading = EncoderRead();

        // update the flags struct
        event_flags.encoder_value_zero = (encoder_reading == 0);

        // wait for event flag that suggests a normal motor command will be sent
        // only then should the encoder value be added to the thread queue
        osEventFlagsWait(inputEventFlagsHandle, 0x0001U, osFlagsWaitAll, osWaitForever);

        // if the encoder value has changed, then put it in the encoder value queue
        if (encoder_reading != old_encoder_reading) {
            osMessageQueuePut(encoderQueueHandle, &encoder_reading, 0U, 0U);
        }

        old_encoder_reading = encoder_reading;

        osThreadYield();
    }
}

// thread to send motor command (torque-control) CAN message
void sendMotorCommandTask(void *argument) {
    uint8_t data_send[DATA_FRAME_LEN];
    osStatus_t status;
    uint16_t encoder_value;

    // velocity is set to unattainable value for motor torque-control mode
    if (event_flags.reverse_enable) {
        velocity.float_value = -100.0;
    } else {
        velocity.float_value = 100.0;
    }

    while (1) {
        // blocks thread waiting for encoder value to be added to queue
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

// thread to send regen command (velocity control) CAN message
void sendRegenCommandTask(void *argument) {
    uint8_t data_send[DATA_FRAME_LEN];

    // velocity is set to zero for regen CAN command
    velocity.float_value = 0.0;

    while (1) {
        // waits for event flag that signals the decision to send a regen command
        osEventFlagsWait(inputEventFlagsHandle, 0x0002U, osFlagsWaitAll, osWaitForever);
        
        // current is linearly scaled with the read regen value
        current.float_value = (float) regen_value / (ADC_MAX - ADC_MIN);

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
    uint32_t flags_to_signal;
    uint8_t battery_soc;

    while (1) {
        // waits for the EXTI ISRs to release semaphore (this only happens when the value changes)
        osSemaphoreAcquire(eventFlagsSemaphoreHandle, osWaitForever);

        // read battery CAN message here
        HAL_CAN_GetRxMessage(&hcan, CAN_FIFO0, &CAN_receive_header, CAN_receive_data);

        battery_soc = CAN_receive_data[0];

        // should send a regen command if the regen is enabled and either of two things is true:
        // 1) the encoder value is zero OR 2) the encoder value and the regen value is not zero 
        // FIXME: this seems a bit wonky, might need to change it in the future
        event_flags.send_regen_command = ((event_flags.regen_enable & event_flags.encoder_value_zero)
                                         | (event_flags.regen_enable & ~event_flags.encoder_value_zero
                                        	& ~event_flags.regen_value_zero)) & (battery_soc < 98);
        event_flags.send_drive_command = !event_flags.send_regen_command;

        // flag_to_signal = 0x0001U -> send normal drive command
        // flag_to_signal = 0x0002U -> send regen drive command
        if (event_flags.send_drive_command) {
            flags_to_signal = 0x0001U;
        } else {
            flags_to_signal = 0x0002U;
        }

        // now use event flags to signal the appropriate flag
        osEventFlagsSet(inputEventFlagsHandle, flags_to_signal);
    }
}

// encoder timer callback function
static void encoderTimerCallback(void *argument) {
    // 0x0001 => it is time to read the encoder
    // TODO: change this to a semaphore to make it easier to understand
    osThreadFlagsSet(readEncoderTaskHandle, 0x0001U);
}

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
