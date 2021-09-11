#include "masterboard_data_collection.h"

CAN_Message_t *txInfo;
uint8_t messageID_table[5] = {0x100, 0x200, 0x300, 0x400, 0x500};                                                                                           //placeholder values -- replace after talking to Matthew
void (*pGetter_fns[5])(BTM_PackData_t *pPackData) = {Voltage_Retrieve, Temperature_Retrieve, SOC_Retrieve, Module_Status_Retrieve, System_Status_Retrieve}; //array of data getter functions

#ifndef INIT_MESSAGE_HEADER
#define INIT_MESSAGE_HEADER

//init CAN Tx Headers for each message type

for (int i = 0; i < CAN_MESSAGE_SERIES_LEN; i++)
{
    (txInfo + i)->TxHeader.StdId = messageID_table[i];
    (txInfo + i)->TxHeader.ExtId = 0;
    (txInfo + i)->TxHeader.IDE = CAN_ID_STD;
    (txInfo + i)->TxHeader.RTR = CAN_RTR_DATA;
    (txInfo + i)->TxHeader.DLC = 8;
    (txInfo + i)->TxHeader.TransmitGlobalTime = 0;
}

//does this work, or would we need to init a entire new CAN_Messasge_t for each data type to send?

#endif

/*
Function Name: tx_data
Function Purpose: 
    Retrieve data using data-type-specific getter functions.
    Tx 8 bytes of data at a time on CAN bus. 

Parameter(s): BTM_PackData_t *pPackData - pointer to struct contining battery-module measurements

Return: void
*/

void tx_data(BTM_PackData_t *pPackData)
{

    int buffer_pos;
    int buffer_end;
    HAL_StatusTypeDef status = HAL_OK;
    int size_of_data;
    int transmission_attempt;

    while (SEND_DATA == 1)
    {

        for (int k = 0; k < CAN_MESSAGE_SERIES_LEN; k++)
        {

            buffer_pos = 0;
            buffer_end = CAN_BYTES_SENT;
            status = HAL_OK;
            size_of_data = sizeof(collected_data) / sizeof(collected_data[0]); //bytes of data needing to be sent
            transmission_attempt = 0;

            *pGetter_fns[k](pPackData); //calling getter function

            while (buffer_end < size_of_data && status == HAL_OK)
            {
                for (int i = 0; i < CAN_BYTES_SENT; i++)
                {
                    (txInfo + k)->dataFrame[i] = voltage_per_module[buffer_pos]; //should be "collected_data[buffer_pos]", change this in individual functions
                    buffer_pos++;
                }

                do
                {
                    status = HAL_CAN_AddTxMessage(&hcan, &((txInfo + k)->TxHeader), (txInfo + k)->dataFrame, &(txInfo + k)->mailbox);
                    transmission_attempt++;
                } while (status != HAL_OK && transmission_attempt < TRANSMISSION_ATTEMPT_MAX);

                //if all three transmission attempts failed
                if (status != HAL_OK)
                {
                    return status;
                }

                buffer_end += CAN_BYTES_SENT; //sent 8 bytes of data
            }
        }
    }
}

void Voltage_Retrieve(BTM_PackData_t *pPackData)
{
    uint8_t collected_data[(RELEVANT_MODULES_PER_STACK * 2)]; //format: [MSBs, LSBs, MSBs, LSBs, ....]

    for (int i = 0; i < BTM_NUM_DEVICES; i++)
    {
        int k = 0;
        for (int j = 0; j < RELEVANT_MODULES_PER_STACK; j++)
        {
            if (pPACKDATA->stack[i].module[j].enable == 1)
            {
                uint16_t module_voltage = pPackData->stack[i].module[j].voltage;
                uint8_t module_voltage_MSBs = (uint8_t)(module_voltage >> 8); //shifting voltage data 8 bits to right, then casting uint16_t to uint8_t, discarding first 8 bits from left
                uint8_t module_voltage_LSBs = (uint8_t)module_voltage;        //casting uint16_t to uint8_t, discarding first 8 bits from left, which were already saved in module_voltage_MSBs

                if (i == 0)
                {
                    collected_data[k] = module_voltage_MSBs;
                    collected_data[k + 1] = module_voltage_LSBs;
                }
                else if (i == 1)
                {
                    collected_data[k + 16] = module_voltage_MSBs;
                    collected_data[k + 17] = module_voltage_LSBs;
                }
                k += 2;
            }
        }
    }
}

void Temperature_Retrieve(BTM_PackData_t *pPackData)
{
    uint8_t collected_data[RELEVANT_MODULES];

    for (int i = 0; i < BTM_NUM_DEVICES; i++)
    {
        for (int j = 0; j < RELEVANT_MODULES_PER_STACK; j++)
        {
            if (pPACKDATA->stack[i].module[j].enable == 1)
            {
                float module_temperature = pPackData->stack[i].module[j].temperature;
                uint8_t module_temperature_int = (uint8_t)(module_temperature * 10); //check to see the value this gives back

                if (i == 0)
                {
                    collected_data[j] = module_temperature_int;
                }
                else if (i == 1)
                {
                    collected_data[j + 16] = module_temperature_int;
                }
            }
        }
    }
}

void SOC_Retrieve(BTM_PackData_t *pPackData)
{
    //same format of retrieval as module-wise temperature
    //SOC for each module (you'll need to do this yourself, look at ltc6813_btm_bal.c for where module-wise SOC info is stored)
}

void Module_Status_Retrieve(BTM_PackData_t *pPackData)
{
    uint8_t collected_data[(RELEVANT_MODULES_PER_STACK * 2)]; //format: [MSBs, LSBs, MSBs, LSBs, ....]

    for (int i = 0; i < BTM_NUM_DEVICES; i++)
    {
        int k = 0;
        for (int j = 0; j < RELEVANT_MODULES_PER_STACK; j++)
        {
            if (pPACKDATA->stack[i].module[j].enable == 1)
            {
                uint16_t module_status = pPackData->stack[i].module[j].status;
                uint8_t module_status_MSBs = (uint8_t)(module_status >> 8);
                uint8_t module_status_LSBs = (uint8_t)module_status;

                if (i == 0)
                {
                    collected_data[k] = module_status_MSBs;
                    collected_data[k + 1] = module_status_LSBs;
                }
                else if (i == 1)
                {
                    collected_data[k + 16] = module_status_MSBs;
                    collected_data[k + 17] = module_status_LSBs;
                }
                k += 2;
            }
        }
    }
}

void System_Status_Retrieve(BTM_PackData_t *pPackData)
{
    uint8_t collected_data[2];
    uint16_t system_status16 = pPackData->PH_status;
    collected_data[0] = (uint8_t)(system_status16 >> 8);
    collected_data[1] = (uint8_t)system_status16;
}
