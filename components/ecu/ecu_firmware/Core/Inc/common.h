/**
 *  @file common.h
 *  @brief Header file for common ecu data structures and module mapping to hardware
 *
 *  @date 2023/11/18
 *  @author Harris Mai (harristmai)
 */

#ifndef __COMMON_H
#define __COMMON_H

#include <stdint.h>
#include <stdbool.h>

/*============================================================================*/
/* STRUCTURES */

typedef union {
    struct {
        bool warning_pack_overdischarge_current : 1;          // WARN_PACK_ODC
        bool warning_pack_overcharge_current : 1;             // WARN_PACK_OC

        bool fault_discharge_overcurrent : 1;         // FLT_DOC
        bool fault_charge_overcurrent : 1;            // FLT_COC

        bool estop : 1;                               // Active High: ESTOP pressed

        bool reset_from_watchdog : 1;

        uint8_t _reserved : 2;
    } bits;
    uint8_t raw;

} ECU_StatusCode_t;

typedef struct{
    
    uint16_t ADC_spare_current; //stores spare current sensor value
    uint16_t ADC_lvs_current; //stores current for the LVS system (mA), max value 30A,
    int32_t ADC_batt_current;//(mA), max 100A

    uint16_t ADC_supp_batt_volt; //stores supplemental battery voltage readings

    uint16_t ADC_batt_curr_offset; //stores supplemental battery voltage offset
    uint16_t ADC_lvs_offset; //stores low voltage system current offset
    uint16_t ADC_spare_curr_offset; //stores spare current source offset

} ECU_ADC_Data_t;

typedef struct{

    ECU_StatusCode_t status;
    ECU_ADC_Data_t adc_data;
    
} ECU_Data_t;

/*============================================================================*/
/* PUBLIC VARIABLES */

extern ECU_Data_t ecu_data;

#endif /* __COMMON_H */