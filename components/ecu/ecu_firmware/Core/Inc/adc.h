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


/*============================================================================*/
/* CONFIGURATION */

#define ADC3_NUM_ANALOG_CHANNELS 3
#define ADC3_BUF_LENGTH_PER_CHANNEL 200
#define ADC3_BUF_LENGTH (ADC3_BUF_LENGTH_PER_CHANNEL * ADC3_NUM_ANALOG_CHANNELS)

#define ADC_MAX_VOLT_READING 3.3
#define ADC_RESOLUTION 4095

/*============================================================================*/
/* PUBLIC VARIABLES */

/*============================================================================*/
/* FUNCTION PROTOTYPES */

int ADC3_getSuppBattVoltage(); //unsigned int?
void ADC3_setSuppBattVoltage(float supp_voltage);

int ADC3_getMotorCurrent();
void ADC3_setMotorCurrent(float motor_current);

int ADC3_getArrayCurrent();
void ADC3_setArrayCurrent(float array_current);

int ADC3_netCurrentOut(int motor_current, int array_current);
void ADC3_processRawReadings(int half, volatile int adc3_buf[], float result[]);

int ADC3_getBusyStatus();
void ADC3_setFaultStatus(int flag_value);

int ADC3_getFaultStatus(); 
void ADC3_setFaultStatus(int flag_value);

#endif /* __ADC_H */
