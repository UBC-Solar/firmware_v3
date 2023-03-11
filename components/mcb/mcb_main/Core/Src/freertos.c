/* USER CODE BEGIN Header */

/**
 * @file    freertos.c
 * @brief   Contains definitions for FreeRTOS tasks.
 * @author  Mihir Nimgade <mihir.nim@outlook.com>
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
#include "tim.h"
#include "encoder.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define ENCODER_QUEUE_MSG_CNT           5               /**< Maximum number of messages allowed in encoder queue. */
#define ENCODER_QUEUE_MSG_SIZE          2               /**< Size of each message in encoder queue (two bytes or `uint16_t`). */

#define DEFAULT_CRUISE_SPEED            10              /**< Speed value that is used when cruise control is first turned on. */

#define BATTERY_REGEN_THRESHOLD         90              /**< Maximum battery percentage at which regenerative braking is allowed to take place. */

#define EVENT_FLAG_UPDATE_DELAY         25              /**< Delay between each time the MCB decides next drive state (in ms).*/
#define ENCODER_READ_DELAY              50              /**< Delay between each time encoder is read (in ms).*/
#define READ_BATTERY_SOC_DELAY          5000            /**< Delay between each time the battery SOC is read from CAN (in ms).*/
#define MOTOR_OVERHEAT_DELAY            300000          /**< Delay between each time the motor temperature is read fro CAN (in ms).*/

#define PEDAL_MAX                       0x64            /* TODO: calibrate this */
#define PEDAL_MIN                       0x00            /* TODO: calibrate this */

#define MOTOR_OVERTEMP_THRESHOLD        150             /**< Maximum allowed motor temperature in celsius. */
#define MOTOR_OVERTEMP_CLR_THRESHOLD    100             /**< Temperature at which MCB will leave the MOTOR_OVERHEAT state */

#define TRUE                            1
#define FALSE                           0

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

osThreadId_t readEncoderTaskHandle;                 /**< Thread handle for readEncoderTask(). */

osThreadId_t updateEventFlagsTaskHandle;            /**< Thread handle for updateEventFlagsTask(). */

osThreadId_t sendMotorCommandTaskHandle;            /**< Thread handle for sendMotorCommandTask(). */
osThreadId_t sendRegenCommandTaskHandle;            /**< Thread handle for sendRegenCommandTask(). */
osThreadId_t sendCruiseCommandTaskHandle;           /**< Thread handle for sendCruiseCommandTask(). */
osThreadId_t sendIdleCommandTaskHandle;             /**< Thread handle for sendIdleCommandTask(). */

osThreadId_t sendNextScreenMessageTaskHandle;       /**< Thread handle for sendNextScreenMessageTask(). */

osThreadId_t receiveBatteryMessageTaskHandle;       /**< Thread handle for receiveBatteryMessageTask(). */

osThreadId_t sendMotorOverheatTaskHandle;

osMessageQueueId_t encoderQueueHandle;              /**< Queue handle for the encoder queue. When new encoder values are read
                                                      from the hardware timer by readEncoderTask(), they are placed inside this queue.
                                                      The values in the queue are read by sendMotorCommandTask().*/

osEventFlagsId_t commandEventFlagsHandle;           /**< Event flags handle for the command event flags object. When the MCB
                                                      changes state, this change is signalled to other RTOS tasks using this event
                                                      flag object. The updateEventFlags() task deals with this RTOS object. */

osSemaphoreId_t nextScreenSemaphoreHandle;          /**< Sempahore handle for the next-screen command. This semaphore is released
                                                      when the NEXT_SCREEN button is pressed. The sendNextScreenMessageTask() task
                                                      acquires this semaphore. */

/**
 *  Enumerates the possible states the MCB can be in.
 */
enum DriveState {
    INVALID = (uint32_t) 0x0000,                /**< Indicates an error state. The MCB should not ever be in this state. */

    IDLE = (uint32_t) 0x0001,                   /**< Indicates an idle state. The MCB will enter this state when the car is stationary */

    NORMAL_READY = (uint32_t) 0x0002,           /**< Indicates a normal state. The MCB will enter this state when neither cruise control is
                                                  on nor regenerative braking. In this state, the pedal encoder directly determines the
                                                  motor current and therefore the throttle provided by the motor. */

    REGEN_READY = (uint32_t) 0x0004,            /**< Indicates that regenerative braking is active. The MCB will enter this state when
                                                  REGEN_EN is enabled and the regen value is more than zero. This state takes priority
                                                  over NORMAL_READY, CRUISE_READY, and IDLE.*/

    CRUISE_READY = (uint32_t) 0x0008,           /**< Indicates that cruise control is active. The MCB will enter this state when CRUISE_EN
                                                  is enabled and the cruise value is more than zero. Pressing the pedal down or pressing
                                                  the brake will exit the cruise control state. */

    MOTOR_OVERHEAT = (uint32_t) 0x0010          /**< Indicates that when the motor is over the maximum temperature. The value is set in the
                                                  sendMotorOverheatTask which checks the incoming motor temperature received over CAN. */

} state;                                        /**< Stores the current MCB state. */

volatile uint16_t encoder_reading = 0x0000;     /**< Stores the encoder reading. */

/* USER CODE END Variables */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

void readEncoderTask (void *argument);

void updateEventFlagsTask (void *argument);

void sendMotorCommandTask (void *argument);
void sendRegenCommandTask (void *argument);
void sendCruiseCommandTask (void *argument);

void sendIdleCommandTask (void *argument);

void receiveBatteryMessageTask (void *argument);

void sendNextScreenMessageTask (void *argument);

void sendMotorOverheatTask (void *argument);

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

    // <---- task allocations ---->

    encoderQueueHandle = osMessageQueueNew(ENCODER_QUEUE_MSG_CNT, ENCODER_QUEUE_MSG_SIZE, &encoderQueue_attributes);

    readEncoderTaskHandle = osThreadNew(readEncoderTask, NULL, &readEncoderTask_attributes);
    updateEventFlagsTaskHandle = osThreadNew(updateEventFlagsTask, NULL, &updateEventFlagsTask_attributes);

    sendMotorCommandTaskHandle = osThreadNew(sendMotorCommandTask, NULL, &sendMotorCommandTask_attributes);
    sendRegenCommandTaskHandle = osThreadNew(sendRegenCommandTask, NULL, &sendRegenCommandTask_attributes);
    sendCruiseCommandTaskHandle = osThreadNew(sendCruiseCommandTask, NULL, &sendCruiseCommandTask_attributes);
    sendIdleCommandTaskHandle = osThreadNew(sendIdleCommandTask, NULL, &sendIdleCommandTask_attributes);

    sendNextScreenMessageTaskHandle = osThreadNew(sendNextScreenMessageTask, NULL, &sendNextScreenTask_attributes);

    receiveBatteryMessageTaskHandle = osThreadNew(receiveBatteryMessageTask, NULL, &receiveBatteryMessageTask_attributes);

    sendMotorOverheatTaskHandle = osThreadNew(sendMotorOverheatTask, NULL, &sendMotorOverheatTask_attributes);

    commandEventFlagsHandle = osEventFlagsNew(NULL);

    nextScreenSemaphoreHandle = osSemaphoreNew(1, 0, NULL);

    // TODO: check that none of the above allocations return NULL

    /* USER CODE END Init */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/**
 * @brief  Reads the pedal quadrature encoder and places the read value in an RTOS queue.
 * @param  argument: Not used
 * @retval None
 */
__NO_RETURN void readEncoderTask(void *argument) {
    static uint16_t old_encoder_reading = 0x0000;

    // TODO: replace with ADC read since potentiometer is being used
    EncoderInit();

    while (1) {
        encoder_reading = __HAL_TIM_GET_COUNTER(&htim1);

        // update the event flags struct
        event_flags.encoder_value_is_zero = (encoder_reading == 0);

        if (encoder_reading != old_encoder_reading) {
            osMessageQueuePut(encoderQueueHandle, &encoder_reading, 0U, 0U);
        }

        old_encoder_reading = encoder_reading;

        osDelay(ENCODER_READ_DELAY);
    }
}

/**
 * @brief  Sends motor command (torque-control) CAN message once encoder value is read and the MCB state is NORMAL_READY.
 * @param  argument: Not used
 * @retval None
 */
__NO_RETURN void sendMotorCommandTask(void *argument) {
    uint8_t data_send[CAN_DATA_LENGTH];
    uint16_t encoder_value;

    osStatus_t queue_status;

    while (1) {
        // check if there's anything in the encoder queue (try-semantics)
        queue_status = osMessageQueueGet(encoderQueueHandle, &encoder_value, NULL, 0U);

        if (queue_status == osOK) {
            // motor current is linearly scaled to pedal position
            current.float_value = (float) encoder_value / (PEDAL_MAX - PEDAL_MIN);
        } else {
            // TODO: send CAN message here that indicates the processor is failing to read the encoder
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

        // send CAN message to motor controller
        HAL_CAN_AddTxMessage(&hcan, &drive_command_header, data_send, &can_mailbox);
    }
}

/**
 * @brief  Sends regen command (velocity control) CAN message once MCB state is REGEN_READY.
 * @param  argument: Not used
 * @retval None
 */
__NO_RETURN void sendRegenCommandTask(void *argument) {
    uint8_t data_send[CAN_DATA_LENGTH];

    while (1) {
        // waits for event flag that signals the decision to send a regen command
        osEventFlagsWait(commandEventFlagsHandle, REGEN_READY, osFlagsWaitAll, osWaitForever);

        // velocity is set to zero for regen according to motor controller documentation
        velocity.float_value = 0.0;

        // current is linearly scaled with the regen value read from the ADC
        current.float_value = (float) regen_value / (ADC_MAX - ADC_MIN);

        // writing data into data_send array which will be sent as a CAN message
        for (int i = 0; i < (uint8_t) CAN_DATA_LENGTH / 2; i++) {
            data_send[i] = velocity.bytes[i];
            data_send[4 + i] = current.bytes[i];
        }

        HAL_CAN_AddTxMessage(&hcan, &drive_command_header, data_send, &can_mailbox);
    }
}

/**
 * @brief  Sends cruise-control command (velocity control) CAN message once MCB state is CRUISE_READY.
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

        HAL_CAN_AddTxMessage(&hcan, &drive_command_header, data_send, &can_mailbox);
    }
}

/**
 * @brief  Sends an idle CAN message when the MCB goes into the IDLE state.
 * @param  argument: Not used
 * @retval None
 */
__NO_RETURN void sendIdleCommandTask (void *argument) {
    uint8_t data_send[CAN_DATA_LENGTH];

    while (1) {
        // waits for event flag that signals the decision to send an idle command
        osEventFlagsWait(commandEventFlagsHandle, IDLE, osFlagsWaitAll, osWaitForever);

        // zeroed since car would not be moving in idle state
        current.float_value = 0.0;
        velocity.float_value = 0.0;

        // writing data into data_send array which will be sent as a CAN message
        for (int i = 0; i < (uint8_t) CAN_DATA_LENGTH / 2; i++) {
            data_send[i] = velocity.bytes[i];
            data_send[4 + i] = current.bytes[i];
        }

        HAL_CAN_AddTxMessage(&hcan, &drive_command_header, data_send, &can_mailbox);
    }
}

/**
 * @brief   Sends CAN message that indicates intention to switch to next page on the driver LCD.
 *          This message is picked up by the DID (driver information display) board.
 * @param   argument: Not used
 * @retval  None
 */
__NO_RETURN void sendNextScreenMessageTask (void *argument) {
    uint8_t data_send[CAN_CONTROL_DATA_LENGTH];

    while (1) {
        // wait for next screen semaphore
        osSemaphoreAcquire(nextScreenSemaphoreHandle, osWaitForever);

        // sets MSB of byte 0 of CAN message to 1 to indicate the next_screen button has been pressed
        data_send[0] = 0x10;

        HAL_CAN_AddTxMessage(&hcan, &screen_cruise_control_header, data_send, &can_mailbox);
    }
}

/**
 * @brief   Decides what state the main control board is in and therefore which thread will send a motor
 *          controller CAN message.
 * @param   argument: Not used
 * @retval  None
 */
__NO_RETURN void updateEventFlagsTask(void *argument) {
    while (1) {
        // order of priorities beginning with most important: motor over heating, regen braking, encoder motor command, cruise control
        if (event_flags.motor_overheat) {
            state = MOTOR_OVERHEAT;
        }
        else if (event_flags.regen_enable && regen_value > 0 && battery_soc < BATTERY_REGEN_THRESHOLD) {
            state = REGEN_READY;
        }
        else if (!event_flags.encoder_value_is_zero && !event_flags.cruise_status) {
            state = NORMAL_READY;
        }
        else if (event_flags.cruise_status && cruise_value > 0 && !event_flags.brake_in) {
            state = CRUISE_READY;
        }
        else {
            state = IDLE;
        }

        // signals the MCB state to other threads
        osEventFlagsSet(commandEventFlagsHandle, state);

        osDelay(EVENT_FLAG_UPDATE_DELAY);
    }
}

/**
 * @brief   Reads battery state-of-charge (SOC) from CAN bus (message ID 0x626). Reading the battery SOC is important
 *          since it helps to decide whether regenerative braking can be applied or not. If the battery SOC is above the
 *          BATTERY_REGEN_THRESHOLD, it is unsafe to activate regenerative braking since doing so might overcharge the
 *          battery.
 *
 * @param   argument: Not used
 * @retval  None
 */
__NO_RETURN void receiveBatteryMessageTask (void *argument) {
    uint8_t battery_msg_data[8];

    while (1) {
        if (HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_RX_FIFO0)) {
            // there are multiple CAN IDs being passed through the filter, check if the message is the SOC
            HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &can_rx_header, battery_msg_data);
            if (can_rx_header.StdId == 0x626) {

                // if the battery SOC is out of range, assume it is at 100% as a safety measure
                if (battery_msg_data[0] < 0 || battery_msg_data[0] > 100) {
                    // TODO: somehow indicate to the outside world that this has happened
                    battery_soc = 100;
                } else {
                    battery_soc = battery_msg_data[0];
                }

            }
        }

        osDelay(READ_BATTERY_SOC_DELAY);
    }
}

/**
 * @brief   Reads motor temperature from CAN bus (message ID 0x50B). Since the the Tritium BMS does not have over-temperature
 *          shutdown, the motor will need to stop sending commands when it reaches a temperature of MAX_MOTOR_TEMPERATURE.
 *
 * @param   argument: Not used
 * @retval  None
 */
__NO_RETURN void sendMotorOverheatTask (void *argument) {
    uint8_t motor_temperature_data[CAN_DATA_LENGTH]; // the motor temperature is bytes [3:0] TODO: this is an assumption

    while (1) {
        if (HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_RX_FIFO0)) {
            // there are multiple CAN IDs being passed through the filter, check if the message is the motor temperature
            HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &can_rx_header, motor_temperature_data);
            if (can_rx_header.StdId == 0x50B) {

                // assign the values from the CAN message into the
                // use the union to convert the 4 bytes to a 32-bit float
                for (int i = 0; i < (uint8_t)CAN_DATA_LENGTH / 2; i++)
                {
                    /*
                     * transfer the incoming bytes LIFO   TODO: test IEEE 754 conversion on hardware
                     * for example, 0xAABBCCDD
                     *        received[0] = 0xDD -> copied[3] = 0xDD
                     *        received[1] = 0xCC -> copied[2] - 0xCC
                     *        received[2] = 0xBB -> copied[1] = 0xBB
                     *        received[3] = 0xAA -> copied[0] - 0xAA
                     */
                    motor_temperature.bytes[i] = motor_temperature_data[CAN_HALF_DATA_LENGTH - 1 - i];
                }

                // if the motor is overheating, stop sending commands
                if (motor_temperature.float_value >= MOTOR_OVERTEMP_THRESHOLD) {
                    // change the state so that sendMotorCommandTask will not run
                    event_flags.motor_overheat = TRUE;
                } 
                else if (motor_temperature.float_value <= MOTOR_OVERTEMP_CLR_THRESHOLD) {
                    event_flags.motor_overheat = FALSE;
                }
            }
        }
        osDelay(MOTOR_OVERHEAT_DELAY);
    }
}

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
