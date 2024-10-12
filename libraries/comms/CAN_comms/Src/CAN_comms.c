/**
 * @file CAN_comms.c
 * @author Kyle Groulx
 * @brief 
 * 
 * @copyright Copyright (c) 2024
 * 
 */


/* Includes */
#include "CAN_comms.h"


/* Private defines */

#define CAN_RX_TASK_STACK_SIZE 512

/* Private varibles */
CAN_comms_config_t CAN_comms_config;

/* Create memory pool */


osMessageQDef(CAN_comms_Rx_queue, CAN_QUEUE_SIZE, CAN_comms_Rx_msg_t);
osMessageQId CAN_comms_Rx_queue;


osThreadId task_CAN_comms_Rx_handle;

/* Private function prototypes */
void task_CAN_comms_Rx(void const* argument);


/**
 * @brief  Initializes CAN communications layer
 * 
 * This function copies the config struct to the global variable and creates the CAN_comms_Rx task.
 * 
 * @attention This function should be called AFTER osKernelInitialize() and BEFORE osKernelStart().
 * 
 * @param  config: Pointer to the config struct
 * @return None
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
    CAN_comms_Rx_queue = osMessageCreate(osMessageQ(CAN_comms_Rx_queue), NULL);

    /* Create tasks */
    osThreadDef(Task_CAN_comms_Rx, task_CAN_comms_Rx, osPriorityNormal, 0, CAN_RX_TASK_STACK_SIZE);
    task_CAN_comms_Rx_handle = osThreadCreate(osThread(Task_CAN_comms_Rx), NULL);
}

/**
 * @brief  Interrupt Service Routine for CAN Rx FIFO
 * 
 * @param  None
 * @return None
 */
void ISR_CAN_comms_Rx()
{
    /* Get CAN message */
    CAN_RxHeaderTypeDef CAN_Rx_header;
    uint8_t CAN_Rx_data[CAN_DATA_SIZE] = {0u};
    if(HAL_OK != HAL_CAN_GetRxMessage(CAN_comms_config.hcan, CAN_comms_config.RX_FIFO, &CAN_Rx_header, CAN_Rx_data))
    {
        return; 
    }

    /* Populate CAN_Rx_msg */
    CAN_comms_Rx_msg_t* CAN_Rx_msg = osPoolAlloc(CAN_Rx_msg_pool);
    CAN_Rx_msg.header = CAN_Rx_header;
    memcpy(CAN_Rx_msg.data, CAN_Rx_data, CAN_DATA_SIZE);

    /* Add CAN message to the queue */
    osMessagePut(CAN_comms_Rx_queue, CAN_Rx_msg, osWaitForever); // TODO: Overflow check
}


void task_CAN_comms_Rx(void const* argument)
{
    /* Infinite loop */
    while (true) 
    {
        /* Wait until there is a message in the queue */ 
        osEvent event = osMessageGet(CAN_comms_Rx_queue, osWaitForever);

        if (event.status == osEventMessage)
        {
            /* Get the CAN message */
            CAN_comms_Rx_msg_t Rx_CAN_msg; = event.value.p;

            /* Call the handle function pointer */
            CAN_comms_config.CAN_comms_handle_Rx(Rx_CAN_msg);
        }
    }
}
