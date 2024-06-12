/**
 *  @file common.h
 *  @brief Header file for common VDS data structures and module mapping to hardware
 *
 *  @date 2024/06/01
 *  @author Makatendeka Chikumbu (makac1896)
 */

#ifndef __COMMON_H
#define __COMMON_H

#include <stdint.h>
#include <stdbool.h>

/*============================================================================*/
/* STRUCTURES */

typedef union {
    struct {
        bool vds_brake_pressure_1: 1; // Flags to indicate integrity of sensor data
        bool vds_brake_pressure_2: 1;
        bool vds_brake_pressure_3: 1;
        bool vds_shock_travel_1: 1;
        bool vds_shock_travel_2: 1;
        bool vds_shock_travel_3: 1;
        bool vds_shock_travel_4: 1;
        bool vds_steering_angle: 1;
        bool adc_fault: 1; // Flag to indicate ADC fault

        uint8_t _reserved  : 2;
    } bits;
    uint8_t raw;

} VDS_StatusCode_t;

typedef struct{
    uint16_t ADC_brake_pressure_1; // mV
    uint16_t ADC_brake_pressure_2; // mV
    uint16_t ADC_brake_pressure_3; // mV
    uint16_t ADC_shock_travel_1; // mV
    uint16_t ADC_shock_travel_2; // mV
    uint16_t ADC_shock_travel_3; // mV
    uint16_t ADC_shock_travel_4; // mV
    uint16_t ADC_steering_angle; // mV
} VDS_ADC_Data_t;


typedef struct{
    VDS_StatusCode_t status;
    VDS_ADC_Data_t adc_data;
} VDS_Data_t;


// Union to simplify process of copying over uint16's into two uint8's for CAN messsages

typedef union {
    struct {
        uint8_t low;
        uint8_t high;
    } bytes;
    uint16_t value;
} ADC_Value_t;


/*============================================================================*/
/* PUBLIC VARIABLES */

extern VDS_Data_t vds_data;
static volatile int ADC1_DMA_in_process_flag; //flag that indicates the DMA interrupt if ADC1 has been called and is in process
static volatile int ADC1_DMA_fault_flag; //flag that indicates the DMA interrupt if ADC1 has been called and is at fault


#endif /* __COMMON_H */
