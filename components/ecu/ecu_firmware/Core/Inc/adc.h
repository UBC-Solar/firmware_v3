/**
 * @file adc.h
 * @brief Header file for ADC interface for supplemental battery
 * and current sensor read from ECU
 * 
 * @date 2021/01/30
 * @author 
 */

#ifndef __ADC_H
#define __ADC_H

#include "stm32f1xx_hal.h"

/*============================================================================*/
/* CONFIGURATION */

// #define ADC_BUFFER_SIZE 100
#define ADC3_NUM_ANALOG_CHANNELS 3
#define ADC3_BUF_LENGTH_PER_CHANNEL 200
#define ADC3_BUF_LENGTH (ADC_BUF_LENGTH_PER_CHANNEL * NUM_ANALOG_CHANNELS)

#define ADC_MAX_VOLT_READING 3.3
#define ADC_RESOLUTION 4095

/*============================================================================*/
/* PUBLIC VARIABLES */

//global variables to store ADC readings
extern int ADC3_supp_batt_volt;
extern int ADC3_motor_current;
extern int ADC3_array_current;

// ADC_HandleTypeDef * ADC_supp_batt_volt;
// ADC_HandleTypeDef * ADC_motor_current; 
// ADC_HandleTypeDef * ADC_array_current;     

// //circular buffers for ADC average readings
// volatile int ADC3_supp_batt_volt_buff[ADC3_BUF_LENGTH];
// volatile int ADC3_motor_current_buff[ADC3_BUF_LENGTH];
// volatile int ADC3_array_current_buff[ADC3_BUF_LENGTH];
// int ADC3_buffer_index;
// int ADC_raw_average;

/*============================================================================*/
/* FUNCTION PROTOTYPES */

// void ADC_init(void);

int ADC3_getSuppBattVoltage();
void ADC3_setSuppBattVoltage(float supp_voltage);

int ADC3_getMotorCurrent();
void ADC3_setMotorCurrent(float motor_current);

int ADC3_getArrayCurrent();
void ADC3_setArrayCurrent(float array_current);

int ADC3_netCurrentOut(int motor_current, int array_current);
void ADC3_processRawReadings(int half, float result[]);

// void ADC_supp_batt_volt_runInterrupt(void);
// void ADC_motor_current_runInterrupt(void);
// void ADC_array_current_runInterrupt(void);


#endif /* __ADC_H */
