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
/* FUNCTION PROTOTYPES */

int ADC_getSuppBattVoltage(unsigned int * supp_voltage);
int ADC_getMotorCurrent(int * motor_current);
int ADC_getArrayCurrent(int * array_current);

#endif /* __ADC_H */
