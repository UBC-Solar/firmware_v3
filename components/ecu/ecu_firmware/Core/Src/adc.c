/**
 * @file adc.c
 * @brief ADC interface for supplemental battery and current sensor read on ECU
 * 
 * These functions need to average out the ADC readings for accuracy.
 * 
 * @date 2021/10/30
 * @author Janith W, Jack K 
 */

#include "adc.h"
#include "stm32f1xx_hal.h"

/*============================================================================*/
/* PRIVATE VARIABLES */

static ADC1_Context_t *adc1_ctx;

/*============================================================================*/
/* PUBLIC FUNCTIONS */

void ADC1_Init(ADC_HandleTypeDef hadc, TIM_HandleTypeDef htim){
  
  adc1_ctx->tim=&htim;
  adc1_ctx->hadc=&hadc;
  adc1_ctx->status=ADC_NO_BUSY;

  HAL_TIM_Base_Start(&htim);
  HAL_ADC_Start_DMA(&hadc, (uint32_t *) adc1_ctx->dma_buf, ADC1_BUF_LENGTH);
  

}

void ADC1_SetReading(ADC1_Channel_List_t adc_channel)
{
  float adc_voltage = adc_reading;

  // Handle out of bound cases
  if (adc_voltage < 0) {
    adc_voltage = 0;
  }
  else if (adc_voltage >= ADC_MAX_RAW) {
    adc_voltage = ADC_MAX_RAW;
  }

  // Convert raw bits to voltage
  adc_voltage = adc_voltage * ADC_VOLTAGE_SCALING * ADC_MAX_VOLT/ADC_MAX_RAW;

  // Scale output data according to channel
  switch (adc_channel)
  {
  case SPAR_CURR_SNS_OFFSET__ADC1_IN5: //Records array and motor current sensor voltage offset in mV
    ADC_spare_curr_offset = 0; //spare current sensor not implemented yet therefore value set to 0
    break;
  
  case SPAR_CURR_SNS__ADC1_IN15:
    ADC_spare_curr = 0; //spare current sensor not implemented yet therefore value set to 0
    break;

  case SUPP_SENSE__ADC1_IN6: //Records supplementary battery voltage in mV
    ADC_supp_batt_volt = (int)(adc_voltage/0.175);
    break;

  case BATT_CURR_SNS_OFFSET__ADC1_IN7:
    ADC_batt_offset = adc_voltage;
    break;
  
  case BATT_CURR_SNS__ADC1_IN14: //Records battery current sensor in A
    ADC_batt_current = 100*(adc_voltage/ADC_VOLTAGE_SCALING-ADC_batt_offset)/0.625; //see HASS100-S datasheet 
    break;
  
  case LVS_CURR_SNS_OFFSET__ADC1_IN8:
    ADC_lvs_offset = adc_voltage;
    break;

  case LVS_CURR_SNS__ADC1_IN9: //Records ECU low voltage current in A
    ADC_lvs_current = (adc_voltage/ADC_VOLTAGE_SCALING-ADC_batt_offset)/0.02525;
    break;

  default:
    break;
  }
}


void ADC1_AverageRawReadings(int half)
{
  int32_t sum[ADC1_NUM_ANALOG_CHANNELS] = {0}; //used for summing in the averaging process
  
  int sample_num = 0; //start index for averaging the ADC readings
  int limit = ADC1_BUF_LENGTH_PER_CHANNEL >> 1; // divide by 2; the end index of the averaging process

  if(half == 1) //start and end indices if averaging the second half of the ADC3's DMA circular buffer
  {
    sample_num = ADC1_BUF_LENGTH_PER_CHANNEL >> 1; 
    limit = ADC1_BUF_LENGTH_PER_CHANNEL;
  }

  // Average the samples
  for(; sample_num < limit; sample_num++) //summing the readings in the averaging process
  {
    
    for(int channel = 0; channel < ADC1_NUM_ANALOG_CHANNELS; channel++)
    {
      //adc3_buf is organized as [supp batt x 200 ... motor curr x 200 ... array curr x 200]
      //when sampling at 100Hz
      sum[channel] += (float)adc1_buf[ADC1_NUM_ANALOG_CHANNELS * sample_num + channel];
    }
  }
  
  //the division process of the averaging
  for(int channel = 0; channel < ADC1_NUM_ANALOG_CHANNELS; channel++)
  {
    //averages for supplemental batery voltage, motor current, and array current
    result[channel] = ((float) sum[channel]) / (ADC1_BUF_LENGTH_PER_CHANNEL >> 1);
  }
}


void ADC1_setFaultStatus(int flag_value)
{
  ADC1_DMA_fault_flag = flag_value;
}


int ADC1_getFaultStatus() 
{
  return ADC1_DMA_fault_flag;
}


void ADC1_setBusyStatus(int flag_value)
{
  ADC1_DMA_in_process_flag = flag_value;
}


int ADC1_getBusyStatus() 
{
  return ADC1_DMA_in_process_flag;
}


/*============================================================================*/
/* DMA-RELATED FUNCTIONS */

void ADC1_averageAndSaveValues(int dma_buf_half)
{
  if (!ADC1_getBusyStatus()){
    ADC1_setBusyStatus(1);
    float result[ADC1_NUM_ANALOG_CHANNELS] = {0};

    ADC1_AverageRawReadings(dma_buf_half, adc1_buf, result);

    for(ADC1_Channel_List_t i = 0; i<ADC1_NUM_ANALOG_CHANNELS; i++){
      ADC1_SetReading(i);
    }

    ADC1_setBusyStatus(0);
  }  
  else
  {
    ADC1_setFaultStatus(1);
  }
}




/*============================================================================*/
/* PRIVATE FUNCTIONS */