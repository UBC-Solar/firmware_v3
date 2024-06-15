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

#include "main.h"

/*============================================================================*/
/* STRUCTURES */

typedef union {
    struct {
        volatile bool vds_brake_pressure_1: 1; // Flags to indicate integrity of sensor data
        volatile bool vds_brake_pressure_2: 1;
        volatile bool vds_brake_pressure_3: 1;
        volatile bool vds_shock_travel_1: 1;
        volatile bool vds_shock_travel_2: 1;
        volatile bool vds_shock_travel_3: 1;
        volatile bool vds_shock_travel_4: 1;
        volatile bool vds_steering_angle: 1;
        volatile bool adc_fault: 1; // Flag to indicate ADC fault

        uint8_t _reserved  : 2;
    } bits;
    uint8_t raw;

} VDS_StatusCode_t;

typedef struct{
    volatile uint16_t ADC_brake_pressure_1; // mV
    volatile uint16_t ADC_brake_pressure_2; // mV
    volatile uint16_t ADC_brake_pressure_3; // mV
    volatile uint16_t ADC_shock_travel_1; // mV
    volatile uint16_t ADC_shock_travel_2; // mV
    volatile uint16_t ADC_shock_travel_3; // mV
    volatile uint16_t ADC_shock_travel_4; // mV
    volatile uint16_t ADC_steering_angle; // mV
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
extern volatile int ADC1_DMA_in_process_flag; //flag that indicates the DMA interrupt if ADC1 has been called and is in process
extern volatile int ADC1_DMA_fault_flag; //flag that indicates the DMA interrupt if ADC1 has been called and is at fault

//keeping track of averages
extern float sum[NUM_ADC_CHANNELS_USED];
extern uint32_t counters[NUM_ADC_CHANNELS_USED];

#endif /* __COMMON_H */
