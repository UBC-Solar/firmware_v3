/**
 * @file CANbus_functions.h
 * @brief
 *
 * Notes
 *
 * @date
 * @author
 */

#ifndef CANBUS_FUNCTION_H_
#define CANBUS_FUNCTION_H_ // this should be the name of the header file, all caps, with suffix as hyphen

#include <stdint.h>
#include "stm32f1xx_hal.h"
#include "ltc6813_btm.h"
#include "analysis.h"
#include <math.h>
#include <stdio.h>
#include <stdbool.h>

/*============================================================================*/
/* CONFIGURABLE PARAMETERS */

#define MAX_CAN_DATAFRAME_BYTES 8
#define NUM_CAN_MESSAGES 7
#define INITIAL_MESSAGE_INDEX 0x622

/*============================================================================*/
/* ENUMERATIONS */

// used in the global array of CAN message structs
typedef enum
{
    INDEX_622 = 0,
    INDEX_623 = 1,
    INDEX_624 = 2,
    INDEX_625 = 3,
    INDEX_626 = 4,
    INDEX_627 = 5,
    INDEX_628 = 6
} CAN_Message_Indicies_t;

/*============================================================================*/
/* STRUCTURES */

typedef struct
{
    CAN_TxHeaderTypeDef tx_header;
    uint8_t data[MAX_CAN_DATAFRAME_BYTES];
    uint32_t mailbox;
    uint8_t enable_send;
} CAN_Tx_Message_t;

typedef struct
{
    CAN_RxHeaderTypeDef rx_header;
    uint8_t data[MAX_CAN_DATAFRAME_BYTES];
} CAN_Rx_Message_t;

/*============================================================================*/
/* FUNCTION PROTOTYPES */

void CAN_InitTxMessages(CAN_Tx_Messages_t txMessageArray[NUM_CAN_MESSAGES]);
void CAN_CompileMessage622(CAN_Tx_Message_t message622, BTM_PackData_t *pack);
void CAN_CompileMessage623(CAN_Tx_Message_t message623, BTM_PackData_t *pack);
void CAN_CompileMessage624(CAN_Tx_Message_t message624, BTM_PackData_t *pack);
void CAN_CompileMessage626(CAN_Tx_Message_t message626, BTM_PackData_t *pack);
void CAN_CompileMessage627(CAN_Tx_Message_t message627, BTM_PackData_t *pack);

#endif