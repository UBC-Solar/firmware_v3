/**
 * @file adc.h
 * @brief Header file for ADC interface for supplemental battery
 * and current sensor read from ECU
 * 
 * @date 2021/10/30
 * @author 
 */

#ifndef __ADC_H
#define __ADC_H

#include "main.h"

/*============================================================================*/
/* CONFIGURATION */

#define ADC3_NUM_ANALOG_CHANNELS 3
#define ADC3_BUF_LENGTH_PER_CHANNEL 200

/*============================================================================*/
/* PUBLIC DEFINITIONS */

typedef enum {
    ADC_BUFFER_HALF_FIRST,
    ADC_BUFFER_HALF_SECOND
} ADC_BufferHalf_t;

/*============================================================================*/
/* FUNCTION PROTOTYPES */

void ADC3_Init(ADC_HandleTypeDef* adc_handle, TIM_HandleTypeDef *trigger_timer_handle);
int ADC3_getSuppBattVoltage(); //unsigned int?
int ADC3_getMotorCurrent();
int ADC3_getArrayCurrent();
int ADC3_netCurrentOut(int motor_current, int array_current);
int ADC3_getBusyStatus();
int ADC3_getFaultStatus();

void ADC3_ConversionCompleteCallback(ADC_BufferHalf_t bufferHalf);
void ADC3_ErrorCallback(void);

#endif /* __ADC_H */
