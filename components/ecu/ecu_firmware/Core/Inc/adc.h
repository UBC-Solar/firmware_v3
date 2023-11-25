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

#include <stdio.h>
#include <stdint.h>
/*============================================================================*/
/* CONFIGURATION */
#define ADC1_NUM_ANALOG_CHANNELS 7
#define ADC1_BUF_LENGTH_PER_CHANNEL 200
#define ADC1_BUF_LENGTH (ADC1_BUF_LENGTH_PER_CHANNEL * ADC1_NUM_ANALOG_CHANNELS)

/*
#define ADC3_NUM_ANALOG_CHANNELS 3 
#define ADC3_BUF_LENGTH_PER_CHANNEL 200
#define ADC3_BUF_LENGTH (ADC3_BUF_LENGTH_PER_CHANNEL * ADC3_NUM_ANALOG_CHANNELS)
*/

#define ADC_MAX_VOLT_READING 3.3
#define ADC_RESOLUTION 4095

#define HASS_50_600_S_VOLTAGE_OFFSET 2.5
#define HASS_SENSOR_DEFAULT_VOLTAGE_OFFSET 1.5
#define HASS_50_600_S_CURRENT_SCALE_FACTOR 80.0
#define HASS_100_S_CURRENT_SCALE_FACTOR 40.0

#define ACS781xLR_VOLTAGE_OFFSET (3.3/2)
#define ACS782xLR_CURRENT_SCALE_FACTOR 26.4

#define SUPP_BATT_VOLTAGE_DIVIDER 11.0
#define ADC_VOLTAGE_SCALING 1000.0 // millivolts

/*============================================================================*/
/* PUBLIC VARIABLES */

/*PRIVATE VARIABLES*/

// static float ADC_spare_curr_offset; //stores spare current source offset
// static float ADC_spare_curr; //stores spare current sensor value
// static int ADC_supp_batt_volt; //stores supplemental battery voltage readings
// static float ADC_batt_offset; //stores supplemental battery voltage offset
// static float ADC_lvs_offset; //stores low voltage system current offset
// static float ADC_lvs_current; //stores current for the LVS system
// static float ADC_batt_current;

static volatile int ADC1_DMA_in_process_flag; //flag that indicates the DMA interrupt if ADC1 has been called and is in process
static volatile int ADC1_DMA_fault_flag; //flag that indicates the DMA interrupt if ADC1 has been called and is at fault
/*============================================================================*/
/*ADC INPUTS*/

typedef enum {
  SPAR_CURR_SNS_OFFSET__ADC1_IN5 = 0,
  SUPP_SENSE__ADC1_IN6,
  BATT_CURR_SNS_OFFSET__ADC1_IN7, 
  LVS_CURR_SNS_OFFSET__ADC1_IN8,
  LVS_CURR_SNS__ADC1_IN9,
  BATT_CURR_SNS__ADC1_IN14,
  SPAR_CURR_SNS__ADC1_IN15, 
}adc_channel_list;


/*============================================================================*/
/* FUNCTION PROTOTYPES */

// float ADC_getOffsetRef_Batt();
// float ADC_getOffsetRef_AM();

// int ADC_getLowVoltageCurrent();
// int ADC_getSuppBattVoltage();

// int ADC_getBatteryCurrent();
// int ADC_getMotorCurrent();
// int ADC_getArrayCurrent();

// int ADC_netCurrentOut();

void ADC1_processRawReadings(int half, volatile uint16_t adc1_buf[], float result[]);

int ADC1_getBusyStatus();
void ADC1_setBusyStatus(int flag_value);

int ADC1_getFaultStatus(); 
void ADC1_setFaultStatus(int flag_value);

int ADC_getReading(int adc_channel);
void ADC_setReading(float adc_reading, adc_channel_list adc_channel);


#endif /* __ADC_H */
