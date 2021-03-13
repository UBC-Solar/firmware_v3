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

#define ADC_BUFFER_SIZE 100

/*============================================================================*/
/* PUBLIC VARIABLES */

ADC_HandleTypeDef * ADC_supp_batt_volt;
ADC_HandleTypeDef * ADC_motor_current; 
ADC_HandleTypeDef * ADC_array_current;     

//circular buffers for ADC average readings
volatile int ADC_supp_batt_volt_buff[ADC_BUFFER_SIZE];
volatile int ADC_motor_current_buff[ADC_BUFFER_SIZE];
volatile int ADC_array_current_buff[ADC_BUFFER_SIZE];
int ADC_buffer_index;
int ADC_raw_average;

/*============================================================================*/
/* FUNCTION PROTOTYPES */

void ADC_init(void);
int ADC_getSuppBattVoltage(unsigned int * supp_voltage);
int ADC_getMotorCurrent(int * motor_current);
int ADC_getArrayCurrent(int * array_current);
int ADC_netCurrentOut(int motor_current, int array_current);

void ADC_supp_batt_volt_runInterrupt(void);
void ADC_motor_current_runInterrupt(void);
void ADC_array_current_runInterrupt(void);


#endif /* __ADC_H */
