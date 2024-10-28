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
#define ID_DELIMITER_CHAR                           '#'
#define CARRIAGE_RETURN_CHAR                        '\r'
#define NEW_LINE_CHAR                               '\n'
#define MASK_4_BITS                                 0xF

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
void RADIO_init();
void RADIO_send_msg_uart(CAN_RxHeaderTypeDef* header, uint8_t* data);


#endif /* __RADIO_H__ */
