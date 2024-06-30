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

//Define the number of ADC samples to be taken
#define NUMBER_ADC_SAMPLES  1024

/*============================================================================*/
/* STRUCTURES */

// Type definition for the ADC sensor averages
typedef struct
{
    // FIFO queue to track the last 1024 samples of each sensor
    struct
    {
        uint16_t ADC_brake_pressure_1[NUMBER_ADC_SAMPLES];
        uint16_t ADC_brake_pressure_2[NUMBER_ADC_SAMPLES];
        uint16_t ADC_brake_pressure_3[NUMBER_ADC_SAMPLES];
        uint16_t ADC_steering_angle[NUMBER_ADC_SAMPLES];
    } values;

    struct
    {
        // Use to track current number of sensors in the FIFO
        uint16_t ADC_brake_pressure_1;
        uint16_t ADC_brake_pressure_2;
        uint16_t ADC_brake_pressure_3;
        uint16_t ADC_steering_angle;
    } counter;

    struct
    {
        // Use to track the sum of all the samples in the FIFO
        uint32_t ADC_brake_pressure_1;
        uint32_t ADC_brake_pressure_2;
        uint32_t ADC_brake_pressure_3;
        uint32_t ADC_steering_angle;
    } previous_sum;
} VDS_ADC_AVERAGES;


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
	uint32_t currentTick_1;
	uint32_t previousTick_1;
	uint32_t currentTick_100;
	uint32_t previousTick_100;
    volatile VDS_StatusCode_t status;
    volatile VDS_ADC_Data_t adc_data;
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

extern volatile VDS_ADC_AVERAGES adc_averages;

//keeping track of averages
extern float sum[NUM_ADC_CHANNELS_USED];
extern uint32_t counters[NUM_ADC_CHANNELS_USED];

extern uint32_t brake_steering_counter;
extern uint32_t shock_travel_counter;
extern uint32_t diagnostic_counter;

#endif /* __COMMON_H */
