/**
 * @file CAN_comms.c
 * 
 * @brief CAN communications layer library.
 * This library contains the implementation for the CAN communications layer, which is responsible
 * for handling the CAN Rx and Tx messages using tasks and queues. This library is designed to be
 * used with the STM32 HAL_CAN library and CMSIS_v2 FreeRTOS.
 * 
 * @attention See README for details to implement this library.
 * 
 */


/* Includes */
#include "CAN_comms.h"


/* Private defines */
#define CAN_RX_TASK_STACK_SIZE 512
#define CAN_TX_TASK_STACK_SIZE 512

#define CAN_RX_QUEUE_SIZE 16
#define CAN_TX_QUEUE_SIZE 16

#define CAN_RX_STRUCT_SIZE sizeof(CAN_comms_Rx_msg_t)
#define CAN_TX_STRUCT_SIZE sizeof(CAN_comms_Tx_msg_t)

#define NUM_CAN_TX_MAILBOXES 3


/* Private varibles */
CAN_comms_config_t CAN_comms_config;
osMessageQueueId_t CAN_comms_Rx_queue;
osMessageQueueId_t CAN_comms_Tx_queue;
osThreadId_t CAN_comms_Rx_task_handle;
osThreadId_t CAN_comms_Tx_task_handle;
osSemaphoreId_t CAN_comms_Tx_mailbox_semaphore;

const osThreadAttr_t CAN_comms_Rx_task_attributes = {
    .name = "CAN_comms_Rx_task",
    .stack_size = CAN_RX_TASK_STACK_SIZE,
    .priority = (osPriority_t) osPriorityNormal,
};

const osThreadAttr_t CAN_comms_Tx_task_attributes = {
    .name = "CAN_comms_Tx_task",
    .stack_size = CAN_TX_TASK_STACK_SIZE,
    .priority = (osPriority_t) osPriorityNormal,
};


/* Private function prototypes */
void CAN_comms_Rx_task(void* argument);
void CAN_comms_Tx_task(void* argument);


/**
 * @brief  Initializes CAN communications layer
 * This function copies the config struct to the global variable and creates the necessary queues, semaphores, and tasks.
 * 
 * @attention This function needs to be called AFTER osKernelInitialize() and BEFORE osKernelStart().
 * 
 * @param  config: Pointer to the config struct
 */
void CAN_comms_init(CAN_comms_config_t* config)
{
    /* Check config is not NULL */
    if (config == NULL)
    {
        return; // TODO: Error handle
    }

    /* Set config */
    CAN_comms_config = *config;

    /* Create queues */
    CAN_comms_Rx_queue = osMessageQueueNew(CAN_RX_QUEUE_SIZE, CAN_RX_STRUCT_SIZE, NULL);
    CAN_comms_Tx_queue = osMessageQueueNew(CAN_TX_QUEUE_SIZE, CAN_TX_STRUCT_SIZE, NULL);


    /* Create Semaphores */
    CAN_comms_Tx_mailbox_semaphore = osSemaphoreNew(NUM_CAN_TX_MAILBOXES, NUM_CAN_TX_MAILBOXES, NULL);

    /* Create tasks */
    CAN_comms_Rx_task_handle = osThreadNew(CAN_comms_Rx_task, NULL, &CAN_comms_Rx_task_attributes);
    CAN_comms_Tx_task_handle = osThreadNew(CAN_comms_Tx_task, NULL, &CAN_comms_Tx_task_attributes);
}


/**
 * @brief  Interrupt Service Routine for CAN Rx
 * This function gets the CAN message from the CAN Rx FIFO and adds it to the CAN_comms_Rx_queue.
 * 
 * @attention This function needs to be added to the CAN Rx FIFO interrupt handler.
 * 
 */
void CAN_comms_Rx_ISR()
{
    /* Get CAN Rx message and populate CAN_comms_Rx_msg_t struct */
    CAN_comms_Rx_msg_t CAN_Rx_msg;
    if(HAL_OK != HAL_CAN_GetRxMessage(CAN_comms_config.hcan, CAN_comms_config.RX_FIFO, &CAN_Rx_msg.header, CAN_Rx_msg.data))
    {
        return; // TODO: Error handle
    }

    /* Add CAN message to the queue */
    if(osOK != osMessageQueuePut(CAN_comms_Rx_queue, &CAN_Rx_msg, 0, 0))
    {
        return; // TODO: Error handle
    }
}


/**
 * @brief Interrupt Service Routine for the CAN mailbox callback function
 * This function frees the semaphore for the CAN Tx mailbox
 * 
 * @attention This function needs to be added to each CAN mailbox complete callback function
 * 
 * @param None
 * @return None
 */
void CAN_comms_mailbox_complete_ISR()
{
    osSemaphoreRelease(CAN_comms_Tx_mailbox_semaphore);
}


/**
 * @brief Task for handling CAN Rx messages
 * This task waits for a message in the CAN_comms_Rx_queue and calls the callback function from the config struct.
 * 
 * @param argument: Unused
 */
void CAN_comms_Rx_task(void* argument)
{
    UNUSED(argument);

    /* Infinite loop */
    for(;;)
    {
        /* Wait until there is a message in the queue */ 
        CAN_comms_Rx_msg_t CAN_comms_Rx_msg;
        if (osOK == osMessageQueueGet(CAN_comms_Rx_queue, &CAN_comms_Rx_msg, NULL, osWaitForever))
        {
            /* Call the handle function pointer */
            CAN_comms_config.CAN_comms_Rx_callback(CAN_comms_Rx_msg);
        }
        else
        {
            return; // TODO: Error handle
        }
    }
}


/**
 * @brief Task for handling CAN Tx messages
 * This task waits for a message in the CAN_comms_Tx_queue and a CAN mailbox semaphore to be released and
 * calls the HAL_CAN_AddTxMessage function.
 * 
 * @param argument: Unused
 * @return None
 */
void CAN_comms_Tx_task(void* argument)
{
    UNUSED(argument);

    /* Infinite loop */
    for(;;)
    {
        /* Wait until there is a message in the queue */ 
        CAN_comms_Tx_msg_t CAN_comms_Tx_msg;
        if (osOK == osMessageQueueGet(CAN_comms_Tx_queue, &CAN_comms_Tx_msg, NULL, osWaitForever))
        {
            /* Wait for a CAN mailbox semaphore to be released */
            osSemaphoreAcquire(CAN_comms_Tx_mailbox_semaphore, osWaitForever);

            uint32_t can_mailbox; // Not used
            if(HAL_OK != HAL_CAN_AddTxMessage(CAN_comms_config.hcan, &CAN_comms_Tx_msg.header, CAN_comms_Tx_msg.data, &can_mailbox))
            {
                return; // TODO: Error handle
            }
        }
        else
        {
            // TODO: Error handle
        }
    }
}


/**
  * @brief  Rx FIFO 0 message pending callback.
  * @param  hcan pointer to a CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @retval None
  */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    UNUSED(hcan);
    CAN_comms_Rx_ISR();
}


/**
  * @brief  Transmission Mailbox 0 complete callback.
  * @param  hcan pointer to a CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @retval None
  */
void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan)
{
    UNUSED(hcan);
    CAN_comms_mailbox_complete_ISR();
}


/**
  * @brief  Transmission Mailbox 1 complete callback.
  * @param  hcan pointer to a CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @retval None
  */
void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *hcan)
{
    UNUSED(hcan);
    CAN_comms_mailbox_complete_ISR();
}


/**
  * @brief  Transmission Mailbox 2 complete callback.
  * @param  hcan pointer to a CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @retval None
  */
void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *hcan)
{
    UNUSED(hcan);
    CAN_comms_mailbox_complete_ISR();
}