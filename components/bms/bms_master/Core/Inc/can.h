/**
 * @file can.h
 * @brief Header file for CAN message management
 *
 * @date October 1, 2022
 * @author Edward Ma, Jung Yi Cau, Mischa Johal
 */

#ifndef INC_CAN_H_
#define INC_CAN_H_

#include <stdint.h>
#include <math.h>
#include <stdbool.h>
#include "stm32f1xx_hal.h"
#include "pack.h"
#include "ltc6813_btm.h"
#include "analysis.h"
#include "soc.h"

#include "../../../../../libraries/github/githash.h"

/*============================================================================*/
/* STRUCTURES */

typedef union {
    struct {
        bool warning_pack_overdischarge_current : 1;          // WARN_PACK_ODC
        bool warning_pack_overcharge_current : 1;             // WARN_PACK_OC

        bool fault_discharge_overcurrent : 1;         // FLT_DOC
        bool fault_charge_overcurrent : 1;            // FLT_COC

        bool reset_from_watchdog : 1;                 // Reset From Independent Watchdog
        bool estop : 1;                               // Active High: ESTOP pressed
        

        uint8_t _reserved : 2;
    } bits;
    uint8_t raw;

} ECU_StatusCode_t;

typedef struct{
    
    int16_t batt_current; //Signed pack current scaled, to get in current in A divide by 65.535, max 100A
    int8_t lvs_current;  
    uint16_t supp_batt_volt; //Unsigned supplemental battery voltage scaled, divide by 1000 to get voltage in V

} ECU_ADC_Data_t;

typedef struct{

    ECU_StatusCode_t status;
    ECU_ADC_Data_t adc_data;

} ECU_Data_t;

/*============================================================================*/
/* FUNCTION PROTOTYPES */

void CAN_Init(CAN_HandleTypeDef *hcan);

void CAN_SendMessage622(Pack_t *pack);
void CAN_SendMessage623(Pack_t *pack);
void CAN_SendMessage624(Pack_t *pack);
void CAN_SendMessage625(Pack_t *pack);
void CAN_SendMessages626(Pack_t *pack);
void CAN_SendMessages627(Pack_t *pack);
void CAN_SendMessages628(Pack_t *pack);
void CAN_SendMessages629(Pack_t *pack);

bool CAN_GetMessage0x450Data(uint32_t *rx_timestamp, ECU_Data_t *ecu_data);

void CAN_RecievedMessageCallback(void);
void CAN_TxCompleteCallback(void);
void CAN_ErrorCallback(void);


#endif // INC_CAN_H_
