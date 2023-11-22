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
        bool warning_pack_overdischarge : 1;          // WARN_PACK_ODC
        bool warning_pack_overcharge : 1;             // WARN_PACK_OC

        bool fault_discharge_overcurrent : 1;         // FLT_DOC
        bool fault_charge_overcurrent : 1;            // FLT_COC

        bool estop : 1;                               // Active High: ESTOP pressed

        uint8_t _reserved : 3;
    } bits;
    uint8_t raw;

} ECU_StatusCode_t;

typedef struct{
    
    int8_t pack_current;
    uint8_t lv_current;
    // float ADC_spare_current; //stores spare current sensor value
    // float ADC_lvs_current; //stores current for the LVS system
    // float ADC_batt_current;

    // int ADC_supp_batt_volt; //stores supplemental battery voltage readings

    //float ADC_batt_offset; //stores supplemental battery voltage offset
    //float ADC_lvs_offset; //stores low voltage system current offset
    //float ADC_am_ref_offset; //stores reference voltage for the array and motor current sensors, from ADC1 
    //float ADC_batt_ref_offset; //stores reference voltage for the battery current sensor, from ADC1
    //float ADC_spare_curr_offset; //stores spare current source offset

} ECU_ADC_Data_t;

typedef struct{

    ECU_StatusCode_t status;
    ECU_ADC_Data_t adc_data;

} ECU_Data_t;

/*============================================================================*/
/* PUBLIC VARIABLES */

extern ECU_Data_t ecu_data;

#endif /* __COMMON_H */