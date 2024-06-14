
/**
 *  @file heartbeat.h
 *  @brief Header file for heartbeat.c. 
 *
 *  @date 2024/13/06
 *  @author Aarjav Jain
 */


#include <stdint.h>
#include "main.h"


#ifndef HEARTBEAT_H
#define HEARTBEAT_H

/* LENGTH CONSTANTS */
#define USER_DIAGNOSTIC_MSG_LENGTH      6                   // User's only use LSB 6 bytes because MSB 2 bytes is heartbeat_counter
#define CAN_DATA_LENGTH                 8

/* CAN STDID CONSTANTS */
#define AMB_HEARTBEAT_STDID             0x201
#define BMS_HEARTBEAT_STDID             0x202
#define DID_HEARTBEAT_STDID             0x203
#define ECU_HEARTBEAT_STDID             0x204
#define OBC_HEARTBEAT_STDID             0x205
#define MDU_HEARTBEAT_STDID             0x206
#define MCB_HEARTBEAT_STDID             0x207
#define MEMORATOR_HEARTBEAT_STDID       0x208
#define TEL_HEARTBEAT_STDID             0x209
#define VDS_HEARTBEAT_STDID             0x20A
#define CAN_NO_EXTID                    0x0000

/* BYTE INDEXING CONSTANTS */
#define COUNTER_BYTE_LO                 6
#define COUNTER_BYTE_HI                 7
#define BYTE_MASK                       0xFF
#define BITS_IN_BYTE                    8
#define DATA_START_INDEX                0
#define DATA_END_INDEX                  7

extern uint16_t heartbeat_counter;

void HEARTBEAT_handler(uint32_t can_stdid, uint8_t* user_diagnostic_data, CAN_HandleTypeDef* hcan, uint32_t can_mailbox);

#endif // HEARTBEAT_H
