#ifndef INC_MASTERBOARD_DATA_COLLECTION_H_
#define INC_MASTERBOARD_DATA_COLLECTION_H_

#include "stm32f3xx_hal.h"
#include "ltc6813_btm.h"

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

void tx_data(BTM_PackData_t *pPackData);
void Voltage_Retrieve(BTM_PackData_t *pPackData);
void Temperature_Retrieve(BTM_PackData_t *pPackData);
void SOC_Retrieve(BTM_PackData_t *pPackData);
void Module_Status_Retrieve(BTM_PackData_t *pPackData);
void System_Status_Retrieve(BTM_PackData_t *pPackData);

#define RELEVANT_MODULES 32
#define RELEVANT_MODULES_PER_STACK 16
#define BYTES_SENT 8
#define TRANSMISSION_ATTEMPT_MAX 3
#define BTM_NUM_DEVICES 2U
#define BTM_NUM_MODULES 18
#define MESSAGE_SERIES_LEN 5
#define SEND_DATA 1 //true

#define DATA_BYTES_MODULE 2

typedef struct
{
    CAN_TxHeaderTypeDef TxHeader;
    uint8_t dataFrame[8];
    uint32_t mailbox = 0;
} CAN_Message_t;

#endif