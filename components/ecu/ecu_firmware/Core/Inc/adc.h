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
#define ADC1_NUM_ANALOG_CHANNELS 8
#define ADC1_BUF_LENGTH_PER_CHANNEL 200
#define ADC1_BUF_LENGTH (ADC1_BUF_LENGTH_PER_CHANNEL * ADC1_NUM_ANALOG_CHANNELS)

#define ADC_MAX_VOLT_READING 3.3
#define ADC_RESOLUTION 4095

#define HASS100S_STD_DEV 1381.049759 // (mA) add this value to sensor's current scaling
// for details, see https://ubcsolar.monday.com/boards/3313680328/pulses/3328157336/posts/2752255843
#define HASS100S_INTERNAL_OFFSET -2551.5 // (mA) see spreadsheet in BMS BOM for sensor default offset characterization
// add this value to standard current measurement to get correct current
#define HASS100S_VOLTAGE_OFFSET 8 // (mV) subtract from ADC measurement of BATT_CURR_SENSE pin to get accurate current reading, see https://ubcsolar26.monday.com/boards/7524367629/pulses/7524367868

#define SUPP_VOLT_DIVIDER_SCALING 0.2249408050513023 //divide ADC reading by this value to get actual SUPP voltage

#define SUPP_BATT_VOLTAGE_DIVIDER 11.0
#define ADC_VOLTAGE_SCALING 1000.0 // millivolts

/*============================================================================*/
/* PUBLIC VARIABLES */

/*PRIVATE VARIABLES*/

static volatile int ADC1_DMA_in_process_flag; //flag that indicates the DMA interrupt if ADC1 has been called and is in process
static volatile int ADC1_DMA_fault_flag; //flag that indicates the DMA interrupt if ADC1 has been called and is at fault
/*============================================================================*/
/*ADC INPUTS*/

typedef enum {
  OD_REF_SENSE__ADC1_IN5 = 0,
  SUPP_SENSE__ADC1_IN6,
  PACK_CURRENT_OFFSET_SENSE__ADC1_IN7, 
  LVS_CURRENT_OFFSET_SENSE__ADC1_IN8,
  LVS_CURRENT_SENSE__ADC1_IN9,
  PACK_CURRENT_SENSE__ADC1_IN14,
  T_AMBIENT_SENSE__ADC1_IN15,
  OC_REF_SENSE__ADC1_IN13
}adc_channel_list;


/*============================================================================*/
/* FUNCTION PROTOTYPES */

void ADC1_processRawReadings(int half, volatile uint16_t adc1_buf[], float result[]);

int ADC1_getBusyStatus();
void ADC1_setBusyStatus(int flag_value);

int ADC1_getFaultStatus(); 
void ADC1_setFaultStatus(int flag_value);

int ADC_getReading(int adc_channel);
void ADC_setReading(float adc_reading, adc_channel_list adc_channel);


#endif /* __ADC_H */
