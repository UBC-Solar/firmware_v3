
/**
 *  @file radio.h
 *  @brief header file for radio.c. Define buffer lengths
 *
 *  @date 2023/03/18
 *  @author Aarjav Jain
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RADIO_TRANSNIT_H__
#define __RADIO_TRANSNIT_H__

#include "can.h"
#include "usart.h"
#include "rtc.h"
#include "xbee.h"


/* RADIO BUFFER BYTE LENGTHS */
#define CAN_BUFFER_LEN                      24
#define GPS_MESSAGE_LEN                     200

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

/*XBee API Packet CONSTANTS*/
#define API_BUFFER_SIZE 					300 			    //number of bytes in api_buffer array
#define API_MAX_MSGS  						10 					//number of messages in each api packet
#define API_OVERHEAD_SIZE 					20 					//number of bytes in api Overhead







void RADIO_tx_CAN_msg(CAN_Radio_msg_t *tx_CAN_msg);

#endif /* __RADIO_H__ */
