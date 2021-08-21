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
#define ENCODER_QUEUE_MSG_SIZE  2       /* 2 bytes (uint16_t) */

#define DEFAULT_CRUISE_SPEED    10      /* To be edited */
#define CRUISE_MAX              100
#define CRUISE_MIN              0

#define IDLE                    (uint32_t) 0x0000
#define NORMAL_READY            (uint32_t) 0x0001
#define REGEN_READY             (uint32_t) 0x0002
#define CRUISE_READY            (uint32_t) 0x0004

#define CAN_FIFO0               0
#define CAN_FIFO1               1
#define DATA_LENGTH             8

#define INIT_EVENT_FLAGS_SEMAPHORE_VAL  0
#define MAX_EVENT_FLAGS_SEMAPHORE_VAL   1

#define PEDAL_MAX               0xD0
#define PEDAL_MIN               0x0F

#define ENCODER_READ_DELAY      100
#define READ_BATTERY_SOC_DELAY  5000

/* USER CODE END PD */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN Variables */

osThreadId_t readEncoderTaskHandle;
osThreadId_t updateEventFlagsTaskHandle;

osThreadId_t sendMotorCommandTaskHandle;
osThreadId_t sendRegenCommandTaskHandle;
osThreadId_t sendCruiseCommandTaskHandle;
osThreadId_t sendNextScreenTaskHandle;

osThreadId_t receiveBatteryMessageTaskHandle;

osMessageQueueId_t encoderQueueHandle;

osEventFlagsId_t commandEventFlagsHandle;
osSemaphoreId_t eventFlagsSemaphoreHandle;

/* USER CODE END Variables */

/* Private function prototypes -----------------------------------------------*/

/* USER CODE BEGIN FunctionPrototypes */

void readEncoderTask(void *argument);

void updateEventFlagsTask(void *argument);

void sendMotorCommandTask(void *argument);
void sendRegenCommandTask(void *argument);
void sendCruiseCommandTask (void *argument);
void sendNextScreenTask (void *argument);


void receiveBatteryMessageTask (void *argument);

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
    updateEventFlagsTaskHandle = osThreadNew(updateEventFlagsTask, NULL, &updateEventFlagsTask_attributes);

    sendMotorCommandTaskHandle = osThreadNew(sendMotorCommandTask, NULL, &sendMotorCommandTask_attributes);
    sendRegenCommandTaskHandle = osThreadNew(sendRegenCommandTask, NULL, &sendRegenCommandTask_attributes);
    sendCruiseCommandTaskHandle = osThreadNew(sendCruiseCommandTask, NULL, &sendCruiseCommandTask_attributes);
    sendNextScreenTaskHandle = osThreadNew(sendNextScreenTask, NULL, &sendNextScreenTask_attributes);
    receiveBatteryMessageTaskHandle = osThreadNew(receiveBatteryMessageTask, NULL, &receiveBatteryMessageTask_attributes);

    // TODO: threads to add - send MCB status message over CAN, read in car speed from CAN bus, transmit "next screen" CAN message

    // <----- Event flag object handles ----->

    commandEventFlagsHandle = osEventFlagsNew(NULL);

    // <----- Semaphore object handles ----->

    eventFlagsSemaphoreHandle = osSemaphoreNew(MAX_EVENT_FLAGS_SEMAPHORE_VAL, INIT_EVENT_FLAGS_SEMAPHORE_VAL, NULL);

  /* USER CODE END Init */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/**
  * @brief  reads the encoder and places the value in the encoder queue every OS tick
  * @param  argument: Not used
  * @retval None
  */
__NO_RETURN void readEncoderTask(void *argument) {
    static uint16_t old_encoder_reading = 0x0000;
    static uint16_t encoder_reading = 0x0000;

    EncoderInit();

    while (1) {
        encoder_reading = EncoderRead();

        // update the event flags struct
        event_flags.encoder_value_is_zero = (encoder_reading == 0);
        
        osSemaphoreRelease(eventFlagsSemaphoreHandle);

        if (encoder_reading != old_encoder_reading) {
            osMessageQueuePut(encoderQueueHandle, &encoder_reading, 0U, 0U);
        }

        old_encoder_reading = encoder_reading;

        osDelay(ENCODER_READ_DELAY)
    }
}

/**
  * @brief  sends motor command (torque-control) CAN message once encoder value is read and a NORMAL_STATE flag is signalled
  * @param  argument: Not used
  * @retval None
  */
__NO_RETURN void sendMotorCommandTask(void *argument) {
    uint8_t data_send[CAN_DATA_LENGTH];
    uint16_t encoder_value;

    osStatus_t queue_status;

    while (1) {
        // blocks thread waiting for encoder value to be added to queue
        queue_status = osMessageQueueGet(encoderQueueHandle, &encoder_value, NULL, 0U);

        if (queue_status == osOK) {
            // current is linearly scaled to pedal position
            current.float_value = (float) encoder_value / (PEDAL_MAX - PEDAL_MIN);
        } else {
            // TODO: could maybe output to UART for debugging or even send a CAN message
            osThreadYield();
        }

        osEventFlagsWait(commandEventFlagsHandle, NORMAL_READY, osFlagsWaitAll, osWaitForever);

        // velocity is set to unattainable value for motor torque-control mode
        if (event_flags.reverse_enable) {
            velocity.float_value = -100.0;
        } else {
            velocity.float_value = 100.0;
        }

        // writing data into data_send array which will be sent as a CAN message
        for (int i = 0; i < (uint8_t) CAN_DATA_LENGTH / 2; i++) {
            data_send[i] = velocity.bytes[i];
            data_send[4 + i] = current.bytes[i];
        }

        HAL_CAN_AddTxMessage(&hcan, &drive_command_header, data_send, &CAN_mailbox);
    }
}

/**
  * @brief  sends regen command (velocity control) CAN message 
  * @param  argument: Not used
  * @retval None
  */
__NO_RETURN void sendRegenCommandTask(void *argument) {
    uint8_t data_send[CAN_DATA_LENGTH];

    while (1) {
        // waits for event flag that signals the decision to send a regen command
        osEventFlagsWait(commandEventFlagsHandle, REGEN_READY, osFlagsWaitAll, osWaitForever);

        // velocity is set to zero for regen CAN command
        velocity.float_value = 0.0;

        // current is linearly scaled with the read regen value
        current.float_value = (float) regen_value / (ADC_MAX - ADC_MIN);

        // writing data into data_send array which will be sent as a CAN message
        for (int i = 0; i < (uint8_t) CAN_DATA_LENGTH / 2; i++) {
            data_send[i] = velocity.bytes[i];
            data_send[4 + i] = current.bytes[i];
        }

        HAL_CAN_AddTxMessage(&hcan, &drive_command_header, data_send, &CAN_mailbox);
    }
}

/**
  * @brief  sends cruise-control command (velocity control) CAN message 
  * @param  argument: Not used
  * @retval None
  */
__NO_RETURN void sendCruiseCommandTask (void *argument) {
    uint8_t data_send[CAN_DATA_LENGTH];

    while (1) {
        // waits for event flag that signals the decision to send a cruise control command
        osEventFlagsWait(commandEventFlagsHandle, CRUISE_READY, osFlagsWaitAll, osWaitForever);

        // current set to maximum for a cruise control message
        current.float_value = 100.0;

        // set velocity to cruise value
        velocity.float_value = (float) cruise_value;

        // writing data into data_send array which will be sent as a CAN message
        for (int i = 0; i < (uint8_t) CAN_DATA_LENGTH / 2; i++) {
            data_send[i] = velocity.bytes[i];
            data_send[4 + i] = current.bytes[i];
        }

        HAL_CAN_AddTxMessage(&hcan, &drive_command_header, data_send, &CAN_mailbox);
    }
}

/**
  * @brief  sends next_screen command (to switch telemetry output) CAN message 
  * @param  argument: Not used
  * @retval None
  */
__NO_RETURN void sendNextScreenTask (void *argument) {
    uint8_t data_send[CAN_DATA_LENGTH];

    while (1) {
        // waits for event flag that signals the decision to send a cruise control command
        osSemaphoreAcquire(eventFlagsSemaphoreHandle, osWaitForever);
        // current set to maximum for a cruise control message
        if (event_flags.next_screen) {
            data_send = 0x01;
            HAL_CAN_AddTxMessage(&hcan, &screen_cruise_control_header, data_send, &CAN_mailbox);
            event_flags.next_screen = 0;
        }

        

        
    }
}


/**
  * @brief  updates specific flags to decide which thread will send a CAN message
  * @param  argument: Not used
  * @retval None
  */
__NO_RETURN void updateEventFlagsTask(void *argument) {
    while (1) {
        // waits for the event flags struct to change
        osSemaphoreAcquire(eventFlagsSemaphoreHandle, osWaitForever);

        // should send a regen command if the regen is enabled and either of two things is true:
        // 1) the encoder value is zero OR 2) the encoder value and the regen value is not zero 

        // order of priorities beginning with most important: regen braking, encoder motor command, cruise control
        if (event_flags.regen_enable && regen_value > 0) {
            osEventFlagsSet(commandEventFlagsHandle, REGEN_READY);
        }
        else if (!event_flags.encoder_value_is_zero && !event_flags.cruise_status) {
            osEventFlagsSet(commandEventFlagsHandle, NORMAL_READY);
        }
        else if (event_flags.cruise_status && cruise_value > 0) {
            osEventFlagsSet(commandEventFlagsHandle, CRUISE_READY);
        }
        // only want to idle after braking when exiting cruise mode
        else if (event_flags.brake_in && event_flags.cruise_status){
            osEventFlagsSet(commandEventFlagsHandle, IDLE);
        }
        else {
            osEventFlagsSet(commandEventFlagsHandle, IDLE);
        }
        
    }
}

/**
  * @brief  Reads battery SOC from CAN bus (message ID 0x626, byte 0)
  * @param  argument: Not used
  * @retval None
  */
__NO_RETURN void receiveBatteryMessageTask (void *argument) {
    uint8_t battery_msg_data[8];

    while (1) {
        if (HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_FIFO0)) {
            // filtering for 0x626 ID done in hardware 
            HAL_CAN_GetRxMessage(&hcan, CAN_FIFO0, &CAN_receive_header, battery_msg_data);

            // if the battery SOC is out of range, assume it is at 100% as a safety measure
            if (battery_msg_data[0] < 0 || battery_msg_data[0] > 100) {
                // TODO: somehow indicate to the outside world that this has happened
                battery_soc = 100;
            } else {
                battery_soc = battery_msg_data[0];
            }
        }

        osDelay(READ_BATTERY_SOC_DELAY)
    }
}

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
