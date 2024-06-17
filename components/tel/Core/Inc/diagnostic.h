/**
 *  @file diagnostic.h
 *  @brief Header file for diagnostic.c. 
 *
 *  @date 2024/13/06
 *  @author Aarjav Jain
 */


#include <stdint.h>
#include "main.h"

#ifndef DIAGNOSTIC_H
#define DIAGNOSTIC_H

/* LENGTH CONSTANTS */
#define USER_DIAGNOSTIC_MSG_LENGTH      6                   // User's only use LSB 6 bytes because MSB 2 bytes is DIAGNOSTIC_counter
#define CAN_DATA_LENGTH                 8

/* CAN STDID CONSTANTS */
#define AMB_DIAGNOSTIC_STDID             0x301
#define BMS_DIAGNOSTIC_STDID             0x302
#define DID_DIAGNOSTIC_STDID             0x303
#define ECU_DIAGNOSTIC_STDID             0x304
#define MDU_DIAGNOSTIC_STDID             0x305
#define MCB_DIAGNOSTIC_STDID             0x306
#define MEMORATOR_DIAGNOSTIC_STDID       0x307
#define TEL_DIAGNOSTIC_STDID             0x308
#define VDS_DIAGNOSTIC_STDID             0x309
#define CAN_NO_EXTID                     0x0000

/* BYTE INDEXING CONSTANTS */
#define COUNTER_BYTE_LO                 0
#define COUNTER_BYTE_HI                 1
#define BYTE_MASK                       0xFF
#define BITS_IN_BYTE                    8
#define DATA_START_INDEX                0
#define DATA_END_INDEX                  7

void DIAGNOSTIC_handler(uint32_t can_stdid, uint8_t* user_diagnostic_data, CAN_HandleTypeDef* hcan, uint32_t can_mailbox);

#endif // DIAGNOSTIC_H
