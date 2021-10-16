#include "masterboard_data_collection.h"

#ifndef INIT_MESSAGE_HEADER
#define INIT_MESSAGE_HEADER

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
    uint8_t collected_data[(RELEVANT_MODULES_PER_STACK * 2)] = {NULL};

    CAN_Message_t txInfo;
    uint8_t messageID_table[5] = {0x100, 0x200, 0x300, 0x400, 0x500}; //update these with Matthew's updated CAN protocol
    //order: voltage, temperature, soc, module status, system status

    //general txInfo paramaters
    txInfo.TxHeader.ExtId = 0;
    txInfo.TxHeader.IDE = CAN_ID_STD;
    txInfo.TxHeader.RTR = CAN_RTR_DATA;
    txInfo.TxHeader.DLC = 8;
    txInfo.TxHeader.TransmitGlobalTime = 0;
    /*
    As data starts to flow, message might only recieve 3 modules worth of data
    this would change amount of data CAN expects to be in message
    if there isn't the same amount of data as in DLC, what issues would this cause? 
    */

    int buffer_pos;
    int buffer_end;
    HAL_StatusTypeDef status = HAL_OK;
    int size_of_data;
    int transmission_attempt;

    while (SEND_DATA == 1)
    {

        for (int k = 0; k < MESSAGE_SERIES_LEN; k++)
        {

            buffer_pos = 0;
            buffer_end = 0;
            status = HAL_OK;
            transmission_attempt = 0;

            if (k = 0)
            {
                Voltage_Retrieve(pPackData, collected_data);
                size_of_data = sizeof(collected_data) / sizeof(collected_data[0]); //number of elements in array
                txInfo.TxHeader.StdId = messageID_table[k];
            }
            else if (k = 1)
            {
                Temperature_Retrieve(pPackData, collected_data);
                size_of_data = sizeof(collected_data) / sizeof(collected_data[0]);
                txInfo.TxHeader.StdId = messageID_table[k];
            }
            else if (k = 2)
            {
                SOC_Retrieve(pPackData, collected_data);
                size_of_data = sizeof(collected_data) / sizeof(collected_data[0]);
                txInfo.TxHeader.StdId = messageID_table[k];
            }
            else if (k = 3)
            {
                Module_Status_Retrieve(pPackData, collected_data);
                size_of_data = sizeof(collected_data) / sizeof(collected_data[0]);
                txInfo.TxHeader.StdId = messageID_table[k];
            }
            else
            {
                System_Status_Retrieve(pPackData, collected_data);
                size_of_data = sizeof(collected_data) / sizeof(collected_data[0]);
                txInfo.TxHeader.StdId = messageID_table[k];
            }

            while (buffer_end < size_of_data && status == HAL_OK)
            {
                for (int i = 0; i < BYTES_SENT; i++)
                {
                    txInfo.dataFrame[i] = collected_data[buffer_pos];
                    buffer_pos++;
                }

                do
                {
                    status = HAL_CAN_AddTxMessage(&hcan, &txInfo.TxHeader, txInfo.dataFrame, &txInfo.mailbox);
                    transmission_attempt++;
                } while (status != HAL_OK && transmission_attempt < TRANSMISSION_ATTEMPT_MAX);

                //if all three transmission attempts failed
                if (status != HAL_OK)
                {
                    return status;
                }

                buffer_end += BYTES_SENT; //sent 8 bytes of data
            }
        }
    }
}

void Voltage_Retrieve(BTM_PackData_t *pPackData, uint8_t collected_data[RELEVANT_MODULES_PER_STACK * 2])
{
    //uint8_t collected_data[(RELEVANT_MODULES_PER_STACK * 2)]; //format: [MSBs, LSBs, MSBs, LSBs, ....]

    for (int i = 0; i < BTM_NUM_DEVICES; i++)
    {
        int k = 0;
        for (int j = 0; j < RELEVANT_MODULES_PER_STACK; j++)
        {
            if (pPACKDATA->stack[i].module[j].enable == 1)
            {
                uint16_t module_voltage = pPackData->stack[i].module[j].voltage;
                uint8_t module_voltage_MSBs = (uint8_t)(module_voltage >> 8); //keeping left (upper) half of 16b voltage
                uint8_t module_voltage_LSBs = (uint8_t)module_voltage;        //keeping right (lower) half of 16b voltage

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

void Temperature_Retrieve(BTM_PackData_t *pPackData, uint8_t collected_data[RELEVANT_MODULES_PER_STACK])
{
    //uint8_t collected_data[RELEVANT_MODULES];

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

void SOC_Retrieve(BTM_PackData_t *pPackData, uint8_t collected_data[RELEVANT_MODULES_PER_STACK])
{
    //same format of retrieval as module-wise temperature
    //SOC for each module (you'll need to do this yourself, look at ltc6813_btm_bal.c for where module-wise SOC info is stored)
}

void Module_Status_Retrieve(BTM_PackData_t *pPackData, uint8_t collected_data[RELEVANT_MODULES_PER_STACK * 2])
{
    //uint8_t collected_data[(RELEVANT_MODULES_PER_STACK * 2)]; //format: [MSBs, LSBs, MSBs, LSBs, ....]

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

void System_Status_Retrieve(BTM_PackData_t *pPackData, )
{
    uint8_t collected_data[2];
    uint16_t system_status16 = pPackData->PH_status; //placeholder status
    collected_data[0] = (uint8_t)(system_status16 >> 8);
    collected_data[1] = (uint8_t)system_status16;
}
