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
#define ADC1_NUM_ANALOG_CHANNELS 4
#define ADC1_BUF_LENGTH_PER_CHANNEL 200
#define ADC1_BUF_LENGTH (ADC1_BUF_LENGTH_PER_CHANNEL * ADC1_NUM_ANALOG_CHANNELS)

#define ADC3_NUM_ANALOG_CHANNELS 3 
#define ADC3_BUF_LENGTH_PER_CHANNEL 200
#define ADC3_BUF_LENGTH (ADC3_BUF_LENGTH_PER_CHANNEL * ADC3_NUM_ANALOG_CHANNELS)

#define ADC_MAX_VOLT_READING 3.3
#define ADC_RESOLUTION 4095

#define HASS_50_600_S_VOLTAGE_OFFSET 2.5
#define HASS_SENSOR_DEFAULT_VOLTAGE_OFFSET 1.5
#define HASS_50_600_S_CURRENT_SCALE_FACTOR 80.0
#define HASS_100_S_CURRENT_SCALE_FACTOR 40.0

#define ACS781xLR_VOLTAGE_OFFSET (3.3/2)
#define ACS782xLR_CURRENT_SCALE_FACTOR 26.4

#define SUPP_BATT_VOLTAGE_DIVIDER 11.0
/*============================================================================*/
/* PUBLIC VARIABLES */

/*PRIVATE VARIABLES*/
static float ADC_am_ref_offset; //stores reference voltage for the array and motor current sensors, from ADC1 
static float ADC_batt_ref_offset; //stores reference voltage for the battery current sensor, from ADC1
static int ADC_lvs_current; //stores current for the LVS system, from ADC1
static int ADC_supp_batt_volt; //stores supplemental battery voltage readings, from ADC1
static int ADC_battery_current; //stores current readings, flowing out of the battery, from ADC3
static int ADC_motor_current; //stores current readings, flowing form the motor to the battery, from ADC3
static int ADC_array_current; //stores current readings, flowing from solar arrays to the battery, from ADC3

static volatile int ADC1_DMA_in_process_flag; //flag that indicates the DMA interrupt if ADC1 has been called and is in process
static volatile int ADC1_DMA_fault_flag; //flag that indicates the DMA interrupt if ADC1 has been called and is at fault
static volatile int ADC3_DMA_in_process_flag; //flag that indicates the DMA interrupt if ADC3 has been called and is in process
static volatile int ADC3_DMA_fault_flag; //flag that indicates the DMA interrupt if ADC3 has been called and is at fault
/*============================================================================*/
/*ADC INPUTS*/

typedef enum {
  OFFSET_REF_AM__ADC1_IN0 = 0,
  LVS_CURR_SENSE__ADC1_IN4,
  SUPP_SENSE__ADC1_IN5, 
  OFFSET_REF_BAT__ADC1_IN10, 
  B_SENSE__ADC3_IN1, 
  M_SENSE__ADC3_IN2, 
  A_SENSE__ADC3_IN3
}adc_channel_list;


/*============================================================================*/
/* FUNCTION PROTOTYPES */

float ADC_getOffsetRef_Batt();
float ADC_getOffsetRef_AM();

int ADC_getLowVoltageCurrent();
int ADC_getSuppBattVoltage();

int ADC_getBatteryCurrent();
int ADC_getMotorCurrent();
int ADC_getArrayCurrent();

int ADC_netCurrentOut();

void ADC3_processRawReadings(int half, volatile uint32_t adc3_buf[], float result[]);
void ADC1_processRawReadings(int half, volatile uint32_t adc1_buf[], float result[]);

int ADC3_getBusyStatus();
void ADC3_setBusyStatus(int flag_value);

void ADC3_setFaultStatus(int flag_value);

int ADC3_getFaultStatus(); 
void ADC3_setFaultStatus(int flag_value);

int ADC_getReading(int adc_channel);
void ADC_setReading(float adc_reading, adc_channel_list adc_channel);


#endif /* __ADC_H */
