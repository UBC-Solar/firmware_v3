/**
 * @file adc.h
 * @brief Header file for ADC interface for supplemental battery
 * and current sensor read from ECU
 * 
 * @date 2021/10/30
 * @author Janith W, Jack K
 */

#ifndef __ADC_H
#define __ADC_H

#include <stdint.h>
#include <main.h>

/*============================================================================*/
/* PUBLIC DEFINES */

#define ADC1_NUM_ANALOG_CHANNELS 7
#define ADC1_BUF_LENGTH_PER_CHANNEL 200
#define ADC1_BUF_LENGTH (ADC1_BUF_LENGTH_PER_CHANNEL * ADC1_NUM_ANALOG_CHANNELS)

#define ADC_MAX_VOLT 3.3
#define ADC_MAX_RAW 4095

#define HASS_SENSOR_DEFAULT_VOLTAGE_OFFSET 1.5
#define HASS_100_S_CURRENT_SCALE_FACTOR 40.0

#define ADC_VOLTAGE_SCALING 1000.0 // millivolts

/*============================================================================*/
/* PRIVATE VARIABLES */

static float ADC_spare_curr_offset; //stores spare current source offset
static float ADC_spare_curr; //stores spare current sensor value
static int ADC_supp_batt_volt; //stores supplemental battery voltage readings
static float ADC_batt_offset; //stores supplemental battery voltage offset
static float ADC_lvs_offset; //stores low voltage system current offset
static float ADC_lvs_current; //stores current for the LVS system
static float ADC_batt_current;

static volatile int ADC1_DMA_in_process_flag; //flag that indicates the DMA interrupt if ADC1 has been called and is in process
static volatile int ADC1_DMA_fault_flag; //flag that indicates the DMA interrupt if ADC1 has been called and is at fault

/*============================================================================*/
/* EXPORTED DATATYPES */

// ADC status codes
typedef enum {
  ADC_NO_BUSY = 0,
  ADC_BUSY,
  ADC_NO_FAULT, 
  ADC_FAULT
} ADC_Status_List_t;

// ADC1
typedef struct{

  ADC_HandleTypeDef *hadc; // pointer to the adc context  
  TIM_HandleTypeDef *tim;

  ADC_Status_List_t status;
  volatile uint16_t dma_buf[ADC1_BUF_LENGTH];
  uint16_t avg_result[ADC1_NUM_ANALOG_CHANNELS];

  ADC1_Channel_List_t channels;

} ADC1_Context_t;

// Channels in enum must be in the same order as configured in the "rank" of the ADC peripheral
typedef enum {
  SPAR_CURR_SNS_OFFSET__ADC1_IN5 = 0,
  SUPP_SENSE__ADC1_IN6,
  BATT_CURR_SNS_OFFSET__ADC1_IN7, 
  LVS_CURR_SNS_OFFSET__ADC1_IN8,
  LVS_CURR_SNS__ADC1_IN9,
  BATT_CURR_SNS__ADC1_IN14,
  SPAR_CURR_SNS__ADC1_IN15, 
} ADC1_Channel_List_t;

/*============================================================================*/
/* FUNCTION PROTOTYPES */

void ADC1_processRawReadings(int half);
void ADC1_setReading(adc1_channel_list_t adc_channel);


#endif /* __ADC_H */
