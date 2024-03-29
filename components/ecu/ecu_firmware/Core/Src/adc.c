/**
 * @file adc.c
 * @brief ADC interface for supplemental battery and current sensor read on ECU
 * 
 * These functions need to average out the ADC readings for accuracy.
 * 
 * @date 2021/10/30
 * @author Forbes Choy 
 */

#include "adc.h"
#include "stm32f1xx_hal.h"
#include "common.h"



/*============================================================================*/
/* PRIVATE FUNCTION PROTOTYPES */

/*============================================================================*/
/* PUBLIC FUNCTIONS */

/**
 * @brief calculates the relevant value for the given ADC channel based on raw value. 
 * 
 * 'adc_reading' is the averaged out raw value from the ADC buffer, and 'adc_channel' is the channel being read,
 * based on the 'adc_channel_list' structure.  
 * 
 * @date 2022/04/08
 * @author Janith Wijekoon
 */

void ADC_setReading(float adc_reading, adc_channel_list adc_channel)
{
  uint16_t adc_voltage = adc_reading;
  if (adc_voltage < 0) adc_voltage = 0;
  else if (adc_voltage >= ADC_RESOLUTION) adc_voltage = ADC_RESOLUTION;
  adc_voltage = adc_voltage * ADC_VOLTAGE_SCALING * ADC_MAX_VOLT_READING/ADC_RESOLUTION;

  switch (adc_channel)
  {
  case SPAR_CURR_SNS_OFFSET__ADC1_IN5: //Records array and motor current sensor voltage offset in mV
    ecu_data.adc_data.ADC_spare_curr_offset = 0; //spare current sensor not implemented yet therefore value set to 0
    break;
  
  case SPAR_CURR_SNS__ADC1_IN15:
    ecu_data.adc_data.ADC_spare_current = 0; //spare current sensor not implemented yet therefore value set to 0
    break;

  case SUPP_SENSE__ADC1_IN6: //Records supplementary battery voltage in mV
    ecu_data.adc_data.ADC_supp_batt_volt = (uint16_t)(adc_voltage/SUPP_VOLT_DIVIDER_SCALING);
    break;

  case BATT_CURR_SNS_OFFSET__ADC1_IN7:
    ecu_data.adc_data.ADC_batt_curr_offset = adc_voltage;
    break;
  
  case BATT_CURR_SNS__ADC1_IN14: //Records battery current sensor in mA
    ecu_data.adc_data.ADC_batt_current = (int32_t)(HASS100S_STD_DEV + HASS100S_INTERNAL_OFFSET + 100*(adc_voltage-ecu_data.adc_data.ADC_batt_curr_offset)/0.625); //see HASS100-S datasheet     
    break;
  
  case LVS_CURR_SNS_OFFSET__ADC1_IN8:
    ecu_data.adc_data.ADC_lvs_offset = adc_voltage;
    break;

  case LVS_CURR_SNS__ADC1_IN9: //Records ECU low voltage current in mA
    ecu_data.adc_data.ADC_lvs_current = (uint16_t)((adc_voltage-ecu_data.adc_data.ADC_batt_curr_offset)/25.25);
    break;

  default:
    break;
  }
}

/**
 * @brief converts the raw readings of ADC1 into ADC voltage readings and
 * averages these values at half of its buffer size per channel, 
 * when the DMA interrupt for ADC1 is called when half of its scan 
 * conversion sequence is completed. These output values will then be converted
 * into am voltage offset, battery voltage offset, lvs current, and supplimentary 
 * battery voltage. 
 * 
 * @param half integer (1 or 0) indicating if the averaging is from the first half (0)
 *             of its circular buffer or its second half (1)
 * @param adc3_buf volatile int array that represents ADC1's DMA circular buffer 
 *                 (200 elements per channel/measurement values; 800 in total. 
 *                  Each time the the ADC is read, the next 4 elements get populated 
 *                  with the am voltage offset, lvs current, supplimentary battery voltage,
 *                  and battery voltage offset in that specific order)
 * @param result float array of size 4, storing the averaged ADC voltages (float) for
 *               each reading.
 * @retval stores the averaged readings of ADC1 to result[]
 */
void ADC1_processRawReadings(int half, volatile uint16_t adc1_buf[], float result[])
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


/**
 * @brief sets the Fault flag in the global variable ADC1_DMA_fault_flag
 * depending if ADC1 attempts to read values in the middle of a DMA interrupt callback process
 * 
 * @param flag_value integer value: 1 is at fault, 0 is not at fault
 * @retval sets ADC1_DMA_fault_flag with flag_value
 */
void ADC1_setFaultStatus(int flag_value)
{
  ADC1_DMA_fault_flag = flag_value;
}

/**
 * @brief Retrieves the fault status of ADC1, stored in global variable
 * ADC1_DMA_fault_flag
 * 
 * @param -
 * @retval returns the global variable ADC1_DMA_fault_flag (int datatype).
 *         1 means at fault; 0 means not at fault
 */
int ADC1_getFaultStatus() 
{
  return ADC1_DMA_fault_flag;
}


/**
 * @brief sets the busy flag in the global variable ADC1_DMA_in_process_flag
 * depending if ADC1 attempts to read values in the middle of a DMA interrupt callback process
 * 
 * @param flag_value integer value: 1 is busy, 0 is not at busy
 * @retval sets ADC1_DMA_in_process_flag with flag_value
 */
void ADC1_setBusyStatus(int flag_value)
{
  ADC1_DMA_in_process_flag = flag_value;
}

/**
 * @brief Retrieves the busy status of ADC1, stored in global variable
 * ADC1_DMA_in_process_flag
 * 
 * @param -
 * @retval returns the global variable ADC1_DMA_in_process_flag (int datatype).
 *         1 means at busy; 0 means not at busy
 */
int ADC1_getBusyStatus() 
{
  return ADC1_DMA_in_process_flag;
}

/*============================================================================*/
/* PRIVATE FUNCTIONS */