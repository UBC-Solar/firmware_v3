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

#include "attributes.h"

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define ENCODER_QUEUE_MSG_CNT   5
#define ENCODER_QUEUE_MSG_SIZE  2    /* 2 bytes (uint16_t) */
#define DEFAULT_CRUISE_SPEED    10     /* To be edited */
#define NUM_MEM_OBJECTS         1

/* USER CODE END PD */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

osThreadId_t readEncoderTaskHandle;
osThreadId_t sendMotorCommandTaskHandle;
osThreadId_t sendRegenCommandTaskHandle;
osThreadId_t sendCruiseCommandTaskHandle;
osThreadId_t updateEventFlagsTaskHandle;

osMemoryPoolId_t eventMemPoolHandle;
event_flags *event_Mem;

osTimerId_t encoderTimerHandle;

osMessageQueueId_t encoderQueueHandle;

osEventFlagsId_t inputEventFlagsHandle;

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

void readRegenTask(void *argument);

void updateEventFlagsTask(void *argument);

void sendMotorCommandTask(void *argument);

void sendRegenCommandTask(void *argument);

void sendCruiseCommandTask (void *argument);

void sendCruiseCommandTask(void *argument);

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
    osStatus_t timer_status;

    encoderQueueHandle = osMessageQueueNew(ENCODER_QUEUE_MSG_CNT, ENCODER_QUEUE_MSG_SIZE, &encoderQueue_attributes);

    defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

    readEncoderTaskHandle = osThreadNew(readEncoderTask, NULL, &readEncoderTask_attributes);
    
    sendMotorCommandTaskHandle = osThreadNew(sendMotorCommandTask, NULL, &sendMotorCommandTask_attributes);

    sendRegenCommandTaskHandle = osThreadNew(sendRegenCommandTask, NULL, &sendRegenCommandTask_attributes);

    sendCruiseCommandTaskHandle = osThreadNew(sendCruiseCommandTask, NULL, &sendCruiseCommandTask_attributes);

    updateEventFlagsTaskHandle = osThreadNew(updateEventFlagsTask, NULL, &updateEventFlagsTask_attributes);


    inputEventFlagsHandle = osEventFlagsNew(NULL);

    eventMemPoolHandle = osMemoryPoolNew(NUM_MEM_OBJECTS, sizeof(event_flags), NULL);
    event_Mem = osMemoryPoolAlloc(eventMemPoolHandle, 0U);


    encoderTimerHandle = osTimerNew(encoderTimerCallback, osTimerPeriodic, NULL, &encoderTimer_attributes);
    
    if (encoderTimerHandle != NULL){
        timer_status = osTimerStart(encoderTimerHandle, ENCODER_TIMER_TICKS);
        if (timer_status != osOK){
            // error
        }
    }
    else{
        // error
    }
    

    /* USER CODE END Init */
}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument) {
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
        // Q: Do we want to wait forever? Or wait set amount of ticks before error
        osThreadFlagsWait(0x0001U, osFlagsWaitAny, osWaitForever);

        encoder_reading = EncoderRead();

        // update the flags struct
        event_Mem->encoder_value_zero = (encoder_reading == 0);

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
    if (event_Mem->reverse_enable) {
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

// thread to send cruise command (velocity-control) CAN message
void sendCruiseCommandTask (void *argument) {
    uint8_t data_send[DATA_FRAME_LEN];

    // velocity is set to zero for regen CAN command
    current.float_value = 100.0;

    while (1) {
        // waits for event flag that signals the decision to send a regen command
        osEventFlagsWait(inputEventFlagsHandle, 0x0003U, osFlagsWaitAll, osWaitForever);
        
        // current is linearly scaled with the read regen value
        velocity.float_value = (float) cruise_value;

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

    while (1) {
        // waits for the EXTI ISRs to set the thread flag
        osThreadFlagsWait(0x0001U, osFlagsWaitAny, osWaitForever);

        // value has been written to the struct, convert this into a number

        // should send a regen command if the regen is enabled and either of two things is true:
        // 1) the encoder value is zero OR 2) the encoder value and the regen value is not zero 

        // self note: for cruise: acc to max, velocity to desired speed --> encoder must be non_zero

        event_Mem->send_regen_command = (event_Mem->regen_enable & event_Mem->encoder_value_zero)
                                         | (event_Mem->regen_enable & ~(event_Mem->encoder_value_zero) 
                                         & ~(event_Mem->regen_value_zero));

        event_Mem->send_cruise_command = event_Mem->cruise_enable & ~(event_Mem->encoder_value_zero);

        event_Mem->send_drive_command = !event_Mem->send_regen_command & !event_Mem->send_cruise_command;

        // flag_to_signal = 0x0001U -> send normal drive command
        // flag_to_signal = 0x0002U -> send regen drive command
        if (event_Mem->send_drive_command) {
            flags_to_signal = 0x0001U;
        } 
        else if (event_Mem->send_regen_command) {
            flags_to_signal = 0x0002U;
        }
        else {
            flags_to_signal = 0x0003U;
        }

        // now use event flags to signal the above number
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
