/**
 *  @file radio.h
 *  @brief header file for radio.c. Define buffer lengths
 * 
 *  Read this monday item for architecture details: https://ubcsolar26.monday.com/boards/7524367653/pulses/7524368294
 */


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RADIO_TRANSNIT_H__
#define __RADIO_TRANSNIT_H__


/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stdint.h"
#include <stdbool.h>


/* DEFINES */
#define RADIO_DATA_LENGTH                           8U      // Made to match CAN format for simplicity
#define RADIO_MSG_TYPEDEF_SIZE                      sizeof(RADIO_Msg_TypeDef)
#define RADIO_QUEUE_SIZE                            60

/* TYPEDEFS */
typedef struct {                                            // Standardized to CAN fields to simplify CAN Rx callback code                          
    uint64_t timestamp;   
    char ID_DELIMETER;
    uint32_t can_id;
    uint8_t data[RADIO_DATA_LENGTH];
    uint8_t data_len;
    char CARRIAGE_RETURN;
    char NEW_LINE;
} __attribute__((packed)) RADIO_Msg_TypeDef;    


/* PROTOTYPES */
void RADIO_Tx_forever();
void RADIO_filter_and_queue_msg(CAN_comms_Rx_msg_t* CAN_comms_Rx_msg);


#endif /* __RADIO_H__ */
