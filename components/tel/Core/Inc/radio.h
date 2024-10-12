/**
 *  @file radio.h
 *  @brief header file for radio.c. Define buffer lengths
 * 
 *  Read this monday item for architecture details: https://ubcsolar26.monday.com/boards/7524367653/pulses/7524368294
 *
 *  @date 2024/10/10
 *  @author Aarjav Jain
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
#define MAX_RX_QUEUE_SIZE                           60U     // See https://ubcsolar26.monday.com/boards/7524367653/pulses/7524368294/posts/3493917111
#define ID_DELIMITER_CHAR                           '#'
#define CARRIAGE_RETURN_CHAR                        '\r'
#define NEW_LINE_CHAR                               '\n'
#define START_OF_ARRAY                              0
#define MASK_4_BITS                                 0xF


/* MACROS */
#define CIRCULAR_INCREMENT_SET(index, max_size) (((index) + 1) % (max_size))


/* TYPEDEFS */
typedef struct {                                            // Standardized to CAN fields to simplify CAN Rx callback code                          
    uint64_t timestamp;   
    char ID_DELIMETER;
    uint32_t can_id_reversed;
    uint8_t data[RADIO_DATA_LENGTH];
    uint8_t data_len;
    char CARRIAGE_RETURN;
    char NEW_LINE;
} __attribute__((packed)) RADIO_Msg_TypeDef;    


typedef struct {
    RADIO_Msg_TypeDef radio_msg;
    bool is_sent;
} RADIO_QueueMsg_TypeDef;


/* PROTOTYPES */
void RADIO_send_msg_uart(double timestamp);	
void RADIO_queue_init();
void RADIO_set_rx_msg(uint32_t can_id, uint8_t* can_data, uint32_t DLC);


#endif /* __RADIO_H__ */
