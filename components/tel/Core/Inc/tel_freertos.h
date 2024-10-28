/**
 *  @file tel_freertos.h
 *  @brief header file for freertos.c
 *
 *  @date 2024/10/27
 *  @author Aarjav Jain
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

#endif /* __RADIO_H__ */
