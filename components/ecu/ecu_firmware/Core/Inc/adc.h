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
/* Global Variables */

ADC_HandleTypeDef * ADC_supp_batt_volt;
ADC_HandleTypeDef * ADC_motor_current; 
ADC_HandleTypeDef * ADC_array_current;     

//circular buffers for ADC average readings
volatile int ADC_supp_batt_volt_buff [BUFFER_SIZE] = {0};
volatile int ADC_motor_current_buff [BUFFER_SIZE] = {0};
volatile int ADC_array_current_buff [BUFFER_SIZE] = {0};
int ADC_buffer_index = 0;
int ADC_raw_average = 0;
int ADC_BUFFER_SIZE = 100;

/*============================================================================*/
/* FUNCTION PROTOTYPES */

int ADC_getSuppBattVoltage(unsigned int * supp_voltage);
int ADC_getMotorCurrent(int * motor_current);
int ADC_getArrayCurrent(int * array_current);

#endif /* __ADC_H */
