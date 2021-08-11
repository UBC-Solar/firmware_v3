#include "masterboard_data_collection.h"

CAN_Message *txInfo;
uint8_t messageID_table[4] = {0x623, 0x627, 0x626, 0x622}; //voltage, temp, pack health, flags

#ifndef INIT_MESSAGE_HEADER
#define INIT_MESSAGE_HEADER

//init CAN Tx Header

for (int i = 0; i < CAN_MESSAGE_SERIES_LEN; i++)
{
    if (i == 4)
    {
        (txInfo + i)->TxHeader.StdId = messageID_table[i - 1];
    }
    else
    {
        (txInfo + i)->TxHeader.StdId = messageID_table[i];
    }
    (txInfo + i)->TxHeader.ExtId = 0;
    (txInfo + i)->TxHeader.IDE = CAN_ID_STD;
    (txInfo + i)->TxHeader.RTR = CAN_RTR_DATA;
    (txInfo + i)->TxHeader.DLC = 8;
    (txInfo + i)->TxHeader.TransmitGlobalTime = 0;
}

#endif

void tx_data(BTM_PackData_t *pPackData, char data_to_send)
{

    while (SEND_DATA)
    {

        if (data_to_send == "v")
        {
            int buffer_pos = 0;
            int buffer_end = CAN_BYTES_SENT;
            HAL_StatusTypeDef status = HAL_OK;

            Voltage_Retrieve(pPackData); //is passing it this way for pointer correct?

            while (buffer_end < (RELEVANT_MODULES * 2) && status == HAL_OK)
            {
                for (int i = 0; i < CAN_BYTES_SENT; i++)
                {
                    txInfo->dataFrame[i] = voltage_per_module[buffer_pos];
                    buffer_pos++;
                }

                int transmission_attempt = 0;
                do
                {
                    status = HAL_CAN_AddTxMessage(CAN_Handle, &((txInfo)->TxHeader), txInfo->dataFrame, &txInfo->mailbox);
                    transmission_attempt++;
                } while (status != HAL_OK && transmission_attempt < TRANSMISSION_ATTEMPT_MAX);

                //if all three transmission attempts failed
                if (status != HAL_OK)
                {
                    return status;
                }

                buffer_end += CAN_BYTES_SENT;
            }
            data_to_send = "t";
        }

        else if (data_to_send == "t")
        {
            int buffer_pos = 0;
            int buffer_end = CAN_BYTES_SENT;
            HAL_StatusTypeDef status = HAL_OK;

            Temperature_Retrieve(pPackData);

            while (buffer_end < RELEVANT_MODULES && status == HAL_OK)
            {
                for (int i = 0; i < CAN_BYTES_SENT; i++)
                {
                    txInfo->dataFrame[i] = temperature_per_module[buffer_pos];
                    buffer_pos++;
                }

                int transmission_attempt = 0;
                do
                {
                    status = HAL_CAN_AddTxMessage(CAN_Handle, &((txInfo + 1)->TxHeader), txInfo->dataFrame, &((txInfo+1)->mailbox);
                    transmission_attempt++;
                } while (status != HAL_OK && transmission_attempt < TRANSMISSION_ATTEMPT_MAX);

                //if all three transmissiona attempts failed
                if (status != HAL_OK)
                {
                    return status;
                }

                buffer_end += CAN_BYTES_SENT;
            }
            data_to_send = "c";
        }

        else if (*data_to_send = "c")
        {
            //SOC send
            data_to_send = "s";
        }
        else if (*data_to_send == "s")
        {
            int buffer_pos = 0;
            int buffer_end = CAN_BYTES_SENT;
            HAL_StatusTypeDef status = HAL_OK;

            Module_Status_Retrieve(pPackData); //is passing it this way for pointer correct?

            while (buffer_end < (RELEVANT_MODULES * 2) && status == HAL_OK)
            {
                for (int i = 0; i < CAN_BYTES_SENT; i++)
                {
                    txInfo->dataFrame[i] = status_per_module[buffer_pos];
                    buffer_pos++;
                }

                int transmission_attempt = 0;
                do
                {
                    status = HAL_CAN_AddTxMessage(CAN_Handle, &((txInfo + 2)->TxHeader), txInfo->dataFrame, &((txInfo+2)->mailbox);
                    transmission_attempt++;
                } while (status != HAL_OK && transmission_attempt < TRANSMISSION_ATTEMPT_MAX);

                //if all three transmission attempts failed
                if (status != HAL_OK)
                {
                    return status;
                }

                buffer_end += CAN_BYTES_SENT;
            }
            data_to_send = "y";
        }
    }

    else if (*data_to_send == "y")
    {
        System_Status_Retrieve(pPackData);

        txInfo->dataFrame[0] = system_status[0];
        txInfo->dataFrame[1] = system_status[1];

        do
        {
            status = HAL_CAN_AddTxMessage(CAN_Handle, &((txInfo + 3)->TxHeader), txInfo->dataFrame, &((txInfo + 3)->mailbox));
            transmission_attempt++;

        } while (status != HAL_OK && transmission_attempt < TRANSMISSION_ATTEMPT_MAX);

        //if all three transmission attempts failed
        if (status != HAL_OK)
        {
            return status;
        }
    }
}

void Voltage_Retrieve(BTM_PackData_t *pPackData)
{
    uint8_t voltage_per_module[(RELEVANT_MODULES_PER_STACK * 2)]; //format: [MSBs, LSBs, MSBs, LSBs, ....]

    for (int i = 0; i < BTM_NUM_DEVICES; i++)
    {
        int k = 0;
        for (int j = 0; j < RELEVANT_MODULES_PER_STACK; j++)
        {
            if (pPACKDATA->stack[i].module[j].enable == 1)
            {
                uint16_t module_voltage = pPackData->stack[i].module[j].voltage;
                uint8_t module_voltage_MSBs = (uint8_t)(module_voltage >> 8);
                uint8_t module_voltage_LSBs = (uint8_t)module_voltage;

                if (i == 0)
                {
                    voltage_per_module[k] = module_voltage_MSBs;
                    voltage_per_module[k + 1] = module_voltage_LSBs;
                }
                else if (i == 1)
                {
                    voltage_per_module[k + 16] = module_voltage_MSBs;
                    voltage_per_module[k + 17] = module_voltage_LSBs;
                }
                k += 2;
            }
        }
    }
}

void Temperature_Retrieve(BTM_PackData_t *pPackData)
{
    uint8_t temperature_per_module[RELEVANT_MODULES];

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
                    temperature_per_module[j] = module_temperature_int;
                }
                else if (i == 1)
                {
                    temperature_per_module[j + 16] = module_temperature_int;
                }
            }
        }
    }
}

void Module_Status_Retrieve(BTM_PackData_t *pPackData)
{
    uint8_t status_per_module[(RELEVANT_MODULES_PER_STACK * 2)]; //format: [MSBs, LSBs, MSBs, LSBs, ....]

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
                    status_per_module[k] = module_status_MSBs;
                    status_per_module[k + 1] = module_status_LSBs;
                }
                else if (i == 1)
                {
                    status_per_module[k + 16] = module_status_MSBs;
                    status_per_module[k + 17] = module_status_LSBs;
                }
                k += 2;
            }
        }
    }
}

void System_Status_Retrieve(BTM_PackData_t *pPackData)
{
    uint8_t system_status[2];
    uint16_t system_status16 = pPackData->PH_status;
    system_status[0] = (uint8_t)(system_status16 >> 8);
    system_status[1] = (uint8_t)system_status16;
}

void SOC_Retrieve(BTM_PackData_t *pPackData)
{
    //same format of retrieval as module-wise temperature
    //SOC for each module (you'll need to do this yourself, look at ltc6813_btm_bal.c for where module-wise SOC info is stored)
}
