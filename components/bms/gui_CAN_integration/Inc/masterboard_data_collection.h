#ifndef INC_MASTERBOARD_DATA_COLLECTION_H_
#define INC_MASTERBOARD_DATA_COLLECTION_H_

#ifndef MASTERBOARD_INCLUDES
#define MASTERBOARD_INCLUDES

#include "stm32f3xx_hal.h"
#include "ltc6813_btm.h"

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#endif

void tx_data(BTM_PackData_t *pPackData, char data_to_send);
void Voltage_Retrieve(BTM_PackData_t *pPackData);
void Temperature_Retrieve(BTM_PackData_t *pPackData);
void Module_Status_Retrieve(BTM_PackData_t *pPackData);
void System_Status_Retrieve(BTM_PackData_t *pPackData);

#define RELEVANT_MODULES 32
#define RELEVANT_MODULES_PER_STACK 16
#define CAN_BYTES_SENT 8
#define TRANSMISSION_ATTEMPT_MAX 3
#define BTM_NUM_DEVICES 2U
#define BTM_NUM_MODULES 18
#define CAN_MESSAGE_SERIES_LEN 5
#define SEND_DATA True

#define DATA_BYTES_MODULE 2

CAN_HandleTypeDef *CAN_Handle; //then you could do &hcan = CAN_Handle;

BTM_PackData_t *pPackData;

typedef struct
{
    CAN_TxHeaderTypeDef TxHeader;
    uint8_t dataFrame[8];
    uint32_t mailbox = 0; //check this
} CAN_Message;

#endif //this means this file will only be included once, so won't get error