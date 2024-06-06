
/**
 *  @file radio_transmit.h
 *  @brief header file for radio_transmit.c. Define buffer lengths
 *
 *  @date 2023/03/18
 *  @author Aarjav Jain
 */


#include "can.h"
#include "usart.h"

/* RADIO BUFFER BYTE LENGTHS */
#define CAN_BUFFER_LEN                      24
#define GPS_MESSAGE_LEN                     200

/* INITIAL CONSTANTS */
#define INITIAL_FLAGS                       0x00
#define FLAG_HIGH                           1
#define FIRST_DATA_BYTE                     0

/* CAN BUFFER INDECIES */
#define TIMESTAMP_INDEX_START               0
#define TIMESTAMP_INDEX_END                 7
#define CAN_MESSAGE_IDENTIFIER_INDEX        8
#define CAN_ID_INDEX_END                    12
#define CAN_DATA_INDEX_START                13
#define CAN_DATA_INDEX_END                  20
#define CAN_DATA_LENGTH_INDEX               21
#define CAN_CARRIAGE_RETURN_INDEX           CAN_BUFFER_LEN - 2
#define CAN_NEW_LINE_INDEX                  CAN_BUFFER_LEN - 1

/* BYTE LOGIC and LENGTHS */
#define BITS_IN_BYTE                        8
#define NUM_STD_ID_BYTES                    2
#define NUM_EXT_ID_BYTES                    4
#define MASK_8_BITS                         0xFF
#define MASK_4_BITS                         0xF

/* MSG CONSTANTS */
#define CAN_MESSAGE_IDENTIFIER              '#'

/* TIMING CONSTANTS */
#define CAN_TRANSMIT_TIMEOUT                1000                // 1 second timeout

void RADIO_TRANSMIT_CAN_msg(CAN_Radio_msg_t *tx_CAN_msg);
