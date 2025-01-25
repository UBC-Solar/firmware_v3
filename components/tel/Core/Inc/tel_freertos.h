/**
 *  @file tel_freertos.h
 *  @brief header file for freertos.c
 */


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TEL_FREERTOS_H__
#define __TEL_FREERTOS_H__


/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* DEFINES */
#define NUM_USART1_TX_SEMAPHORES        1

/* SEMAPHORES */
extern osSemaphoreId_t usart1_tx_semaphore;

/*	QUEUES	*/
extern osMessageQueueId_t CAN_rx_queueHandle;
extern osMessageQueueId_t CAN_tx_queueHandle;


#endif /* __RADIO_H__ */
