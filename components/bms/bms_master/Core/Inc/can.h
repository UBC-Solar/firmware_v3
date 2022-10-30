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
#define NUM_CAN_MESSAGES 4
#define INITIAL_MESSAGE_INDEX 0x622
#define ECU_CURRENT_MSG_ID_MASK 0x450
#define NUM_RX_FIFOS 2
#define MAX_MESSAGES_PER_FIFO 3
#define TOTAL_MODULES_ENABLED 32

/*============================================================================*/
/* ENUMERATIONS */

// used in the global array of CAN message structs
// keeping compatability with the elithion
typedef enum
{
    INDEX_622 = 0,
    INDEX_623,
    INDEX_626,
    INDEX_627
} CAN_Message_Indicies_t;

typedef enum
{
    MSG_NOT_RECIEVED = 0,
    MSG_RECIEVED = 1
} CAN_Rx_Msg_Status_t;

typedef enum
{
    SEND_DISABLED = 0,
    SEND_ENABLED = 1
} CAN_send_status_t;
/*============================================================================*/
/* STRUCTURES */

typedef struct
{
    CAN_TxHeaderTypeDef tx_header;
    uint8_t data[MAX_CAN_DATAFRAME_BYTES];
    uint32_t mailbox;
    CAN_send_status_t enable_send;
} CAN_Tx_Message_t;

typedef struct
{
    CAN_RxHeaderTypeDef rx_header;
    uint8_t data[MAX_CAN_DATAFRAME_BYTES];
    CAN_Rx_Msg_Status_t message_status;
    uint8_t fifo; // fifo from which message was recieved from (may not be needed)
} CAN_Rx_Message_t;

/*============================================================================*/
/* FUNCTION PROTOTYPES */

void CAN_Init(CAN_HandleTypeDef *hcan, CAN_Tx_Messages_t txMessageArray[NUM_CAN_MESSAGES]);
void CAN_CompileMessage622(CAN_Tx_Message_t message622, BTM_PackData_t *pack);
void CAN_CompileMessage623(CAN_Tx_Message_t message623, BTM_PackData_t *pack);
void CAN_CompileMessage624(CAN_Tx_Message_t message624, BTM_PackData_t *pack);
void CAN_CompileMessage625(CAN_Tx_Message_t message625, BTM_PackData_t *pack);
void CAN_CompileMessage627(CAN_Tx_Message_t message627, BTM_PackData_t *pack);
void CAN_RecieveMessages(CAN_HandleTypeDef *hcan, CAN_Rx_Message_t rxMessages[NUM_RX_FIFOS * MAX_MESSAGES_PER_FIFO]);

#endif // CANBUS_FUNCTION_H_