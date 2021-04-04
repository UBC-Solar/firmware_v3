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

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define ENCODER_QUEUE_MSG_CNT   5
#define ENCODER_QUEUE_MSG_SIZE  2    /* 2 bytes (uint16_t) */
#define DEFAULT_CRUISE_SPEED    10     /* To be edited */
#define NUM_MEM_OBJECTS         1

#define CAN_FIFO0 0
#define CAN_FIFO1 1

#define INIT_SEMAPHORE_VALUE 0

/* USER CODE END PD */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN Variables */

osThreadId_t readEncoderTaskHandle;
osThreadId_t sendMotorCommandTaskHandle;
osThreadId_t sendRegenCommandTaskHandle;
osThreadId_t sendCruiseCommandTaskHandle;
osThreadId_t updateEventFlagsTaskHandle;

osMemoryPoolId_t eventMemPoolHandle;

osMessageQueueId_t encoderQueueHandle;

osEventFlagsId_t inputEventFlagsHandle;

osSemaphoreId_t eventFlagsSemaphoreHandle;

input_flags *event_mem;

/* USER CODE END Variables */

/* Private function prototypes -----------------------------------------------*/

/* USER CODE BEGIN FunctionPrototypes */

void readEncoderTask(void *argument);
void readRegenTask(void *argument);
void updateEventFlagsTask(void *argument);
void sendMotorCommandTask(void *argument);
void sendRegenCommandTask(void *argument);
void sendCruiseCommandTask (void *argument);

/* USER CODE END FunctionPrototypes */

void MX_FREERTOS_Init(void);

/**
  * @brief  FreeRTOS initialization
  */
void MX_FREERTOS_Init(void) {
    /* USER CODE BEGIN Init */

    // <----- Queue object handles ----->

    encoderQueueHandle = osMessageQueueNew(ENCODER_QUEUE_MSG_CNT, ENCODER_QUEUE_MSG_SIZE, &encoderQueue_attributes);

    // <----- Thread object handles ----->

    readEncoderTaskHandle = osThreadNew(readEncoderTask, NULL, &readEncoderTask_attributes);

    sendMotorCommandTaskHandle = osThreadNew(sendMotorCommandTask, NULL, &sendMotorCommandTask_attributes);
    sendRegenCommandTaskHandle = osThreadNew(sendRegenCommandTask, NULL, &sendRegenCommandTask_attributes);
    sendCruiseCommandTaskHandle = osThreadNew(sendCruiseCommandTask, NULL, &sendCruiseCommandTask_attributes);

    updateEventFlagsTaskHandle = osThreadNew(updateEventFlagsTask, NULL, &updateEventFlagsTask_attributes);

    // <----- Event flag object handles ----->

    inputEventFlagsHandle = osEventFlagsNew(NULL);

    // <----- Memory pool object handles ----->

    eventMemPoolHandle = osMemoryPoolNew(NUM_MEM_OBJECTS, sizeof(event_flags), NULL);
    event_mem = osMemoryPoolAlloc(eventMemPoolHandle, 0U);

    // <----- Semaphore object handles ----->

    eventFlagsSemaphoreHandle = osSemaphoreNew(1, INIT_SEMAPHORE_VALUE, NULL);

  /* USER CODE END Init */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/**
  * @brief  reads the encoder and places the value in the encoder queue every OS tick
  * @param  argument: Not used
  * @retval None
  */
void readEncoderTask(void *argument) {
    // this variable needs to exist throughout the runtime of the program 
    static uint16_t old_encoder_reading = 0x0000U;
    uint16_t encoder_reading;

    EncoderInit();

    while (1) {
        encoder_reading = EncoderRead();

        // update the flags struct
        event_mem->encoder_value_zero = (encoder_reading == 0);

        osSemaphoreRelease(eventFlagsSemaphoreHandle);

        // if the encoder value has changed, then put it in the encoder value queue
        if (encoder_reading != old_encoder_reading) {
            osMessageQueuePut(encoderQueueHandle, &encoder_reading, 0U, 0U);
        }

        old_encoder_reading = encoder_reading;

        osDelay(ENCODER_TIMER_TICKS);
    }
}

/**
  * @brief  sends motor command (torque-control) CAN message once encoder value is read
  * @param  argument: Not used
  * @retval None
  */
void sendMotorCommandTask(void *argument) {
    uint8_t data_send[DATA_FRAME_LEN];
    osStatus_t status;
    uint16_t encoder_value;

    // velocity is set to unattainable value for motor torque-control mode
    if (event_mem->reverse_enable) {
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
            // FIXME: could maybe output to UART for debugging or even send a CAN message
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

/**
  * @brief  sends regen command (velocity control) CAN message 
  * @param  argument: Not used
  * @retval None
  */
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

/**
  * @brief  sends cruise-control command (velocity control) CAN message 
  * @param  argument: Not used
  * @retval None
  */
__NO_RETURN void sendCruiseCommandTask (void *argument) {
    uint8_t data_send[DATA_FRAME_LEN];

    // current set to maximum for a cruise control message
    current.float_value = 100.0;

    while (1) {
        // waits for event flag that signals the decision to send a cruise control command
        osEventFlagsWait(inputEventFlagsHandle, 0x0003U, osFlagsWaitAll, osWaitForever);

        // set velocity to cruise value
        velocity.float_value = (float) event_mem->cruise_value;

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
        // waits for the event flags struct to change
        osSemaphoreAcquire(eventFlagsSemaphoreHandle, osWaitForever);

        // read battery CAN message here, filtering done in hardware
        HAL_CAN_GetRxMessage(&hcan, CAN_FIFO0, &CAN_receive_header, CAN_receive_data);

        battery_soc = CAN_receive_data[0];

        // if the battery is out of range set it to 100% as a safety measure
        if (battery_soc < 0 || battery_soc > 100) {
            battery_soc = 100;
            // TODO: somehow indicate to the outside world that this has happened
        }

        // should send a regen command if the regen is enabled and either of two things is true:
        // 1) the encoder value is zero OR 2) the encoder value and the regen value is not zero 

        // self note: for cruise: acc to max, velocity to desired speed --> encoder must be non_zero

        // TODO: convert this to a regular conditional, it is too hard to read and might actually negatively
        // affect compiler optimizations
        // TODO: consider replacing with a state machine to remove the possibility of corner cases
        event_mem->send_regen_command = (event_mem->regen_enable & event_mem->encoder_value_zero)
                                         | (event_mem->regen_enable & ~(event_mem->encoder_value_zero)
                                         & ~(event_mem->regen_value_zero)) & (battery_soc < 98);

        event_mem->send_cruise_command = event_mem->cruise_enable & ~(event_mem->encoder_value_zero);

        event_mem->send_drive_command = !event_mem->send_regen_command & !event_mem->send_cruise_command;

        // flag_to_signal = 0x0001U -> send normal drive command
        // flag_to_signal = 0x0002U -> send regen drive command
        // flag_to_signal = 0x0003U -> send cruise control drive command
        if (event_mem->send_drive_command) {
            flags_to_signal = 0x0001U;
        }
        else if (event_mem->send_regen_command) {
            flags_to_signal = 0x0002U;
        }
        else {
            flags_to_signal = 0x0003U;
        }

        // now use event flags to signal the appropriate flag
        osEventFlagsSet(inputEventFlagsHandle, flags_to_signal);
    }
}

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
