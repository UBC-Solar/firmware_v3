/**
 * @file can.c
 * @brief CAN communication functions for ECU
 * 
 * @date 2021/01/30
 * @author Blake Shular (blake-shular)
 */

#include "can.h"

//Double check with Mischa as max pack discharge most likely different from charging
#define MAX_CHARGING_CURRENT 0b10100010100  // 1300 in binary, 130 A 0.1A/bit offset
#define MAX_CHARGING_VOLTAGE 0b10100111100  // 1340 in binary, 134 V 0.1V/bit offset

#define MESSAGE_LENGTH 8
CAN_TxHeaderTypeDef current_message_header = {
    0x450, 0, CAN_ID_STD, CAN_RTR_DATA, 3, DISABLE
};

CAN_TxHeaderTypeDef charger_contraints_modes_header = {
    0x3F4, 0, CAN_ID_STD, CAN_RTR_DATA, MESSAGE_LENGTH, DISABLE
};    

void CAN_send_current(int currentVal) {
    uint32_t begin_tick = HAL_GetTick();
    uint32_t * pTxMailbox = 0;
    HAL_StatusTypeDef status;
    currentVal /= 100;
    uint8_t data[] = {(uint8_t)currentVal, (uint8_t)(currentVal >> 8), 0, 0, 0, 0};
    do {
        status = HAL_CAN_AddTxMessage(CAN_hcan, &current_message_header, data, pTxMailbox);
    } while (status != HAL_OK && HAL_GetTick() - begin_tick <= CAN_TIMEOUT);
}

void CAN_charging_mode(int charger_enables){//check if battery charger has finished initializing
    uint32_t begin_tick = HAL_GetTick(); //What does this do exactly?
    uint32_t * pTxMailbox = 0;
    HAL_StatusTypeDef status;

    uint8_t data[MESSAGE_LENGTH] = {(uint8_t)MAX_CHARGING_CURRENT, (uint8_t)(MAX_CHARGING_CURRENT >> 8),
                                    (uint8_t)MAX_CHARGING_VOLTAGE, (uint8_t)(MAX_CHARGING_VOLTAGE >> 8),
                                    (uint8_t)charger_enables, 0, 0, 0};

    do {
        status = HAL_CAN_AddTxMessage(CAN_hcan, &charger_contraints_modes_header, data, pTxMailbox);
    } while (status != HAL_OK && HAL_GetTick() - begin_tick <= CAN_TIMEOUT);//Why is this timeout 5ms should I change it to 5s in this case as specified in the data sheet
}