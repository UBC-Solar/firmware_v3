/**
 * @file adc.c
 * @brief ADC interface for supplemental battery and current sensor read on ECU
 * 
 * These functions need to average out the ADC readings for accuracy.
 * 
 * @date 2021/10/30
 * @author Forbes Choy, Jack Kelly 
 */

#include "adc.h"
#include "stm32f1xx_hal.h"
#include "common.h"
#include <math.h>

/*============================================================================*/
/* PRIVATE FUNCTION PROTOTYPES */
float volts2temp(uint16_t adc_voltage);
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
  uint16_t adc_voltage = (uint16_t)adc_reading;
  if (adc_voltage < 0) adc_voltage = 0;
  else if (adc_voltage >= ADC_RESOLUTION) adc_voltage = ADC_RESOLUTION;
  adc_voltage = adc_voltage * ADC_VOLTAGE_SCALING * ADC_MAX_VOLT_READING/ADC_RESOLUTION;

  int32_t curr_voltage_error = 0; // Define variable for more readable current reading calculation

  switch (adc_channel)
  {
  case OD_REF_SENSE__ADC1_IN5: // Overdischarge current threshold (mV)
    ecu_data.adc_data.ADC_od_ref = 0;
    break;
  
  case SUPP_SENSE__ADC1_IN6: // Supplemental battery voltage (mV)
    ecu_data.adc_data.ADC_supp_batt_volt = (uint16_t)(adc_voltage/SUPP_VOLT_DIVIDER_SCALING);
    break;

  case PACK_CURRENT_OFFSET_SENSE__ADC1_IN7: // Pack current sensor offset voltage (mV)
    ecu_data.adc_data.ADC_pack_current_offset = adc_voltage;
    break;

  case LVS_CURRENT_OFFSET_SENSE__ADC1_IN8: // LVS current sensor offset voltage (mV)
    ecu_data.adc_data.ADC_lvs_current_sense_offset = adc_voltage;
    break;
  
  case LVS_CURRENT_SENSE__ADC1_IN9: // LVS current supplied to HVDCDC (mA)
    ecu_data.adc_data.ADC_lvs_current = (uint16_t)((adc_voltage-ecu_data.adc_data.ADC_lvs_current_sense_offset)/25.25);
    break;
  
  case PACK_CURRENT_SENSE__ADC1_IN14: // Pack current sense (mA)
    curr_voltage_error = HASS100S_VOLTAGE_ERROR_TERM_CONSTANT + (HASS100S_VOLTAGE_ERROR_TERM_MULTIPLE * adc_voltage); // Error Polynomial, See https://ubcsolar26.monday.com/boards/7524367629/pulses/7524367868/posts/3902002110
    ecu_data.adc_data.ADC_pack_current = (int32_t)(HASS100S_STD_DEV + HASS100S_INTERNAL_OFFSET + 100*(adc_voltage + curr_voltage_error - ecu_data.adc_data.ADC_pack_current_offset) / 0.625); //see HASS100-S datasheet
    break;

  case T_AMBIENT_SENSE__ADC1_IN15: // Ambient controlboard temperature (deg C)
    ecu_data.adc_data.ADC_temp_ambient_sense = (int8_t)volts2temp(adc_voltage);
    break;

  case OC_REF_SENSE__ADC1_IN13: // Overcharge current threshold (mV)
    ecu_data.adc_data.ADC_oc_ref = 0;
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

  if(half == 1) //start and end indices if averaging the second half of the ADC1's DMA circular buffer
  {
    sample_num = ADC1_BUF_LENGTH_PER_CHANNEL >> 1; 
    limit = ADC1_BUF_LENGTH_PER_CHANNEL;
  }

  // Average the samples
  for(; sample_num < limit; sample_num++) //summing the readings in the averaging process
  {
    for(int channel = 0; channel < ADC1_NUM_ANALOG_CHANNELS; channel++)
    {
      // adc1_buf is organized as [supp batt x 200 ... motor curr x 200 ... array curr x 200]
      // when sampling at 100Hz (not sure if that 100Hz number is correct)
      sum[channel] += (float)adc1_buf[ADC1_NUM_ANALOG_CHANNELS * sample_num + channel];
    }
  }
  
  //the division process of the averaging
  for(int channel = 0; channel < ADC1_NUM_ANALOG_CHANNELS; channel++)
  {
    // averages across all ADC samples within respective ADC channel
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

/**
 * @brief Converts a raw thermistor voltage reading into a temperature in degrees celsius
 *
 * @param[in] adc_voltage the thermistor voltage reading to convert
 * @return the signed temperature of the thermistor in degrees celcius
 */
float volts2temp(uint16_t adc_voltage)
{
  const float beta = 3977.0; // TODO: update depending on chosen thermistor
  const float room_temp = 298.15; // 25 degC in kelvin
  const float R_balance = 10000.0; // TODO: measure off-board 10k resistor with DMM before soldering on
  const float R_room_temp = 10000.0; // resistance at room temperature (25C)
  float Vs = 3.3; // 3V3 ideally, measure under realistic operating conditions
  float R_therm = 0.0;
  float temp_kelvin = 0.0;
  float Vout = 0.0; // volts

  Vout = adc_voltage / ADC_VOLTAGE_SCALING;

  R_therm = R_balance * (Vout/(Vs-Vout));
  temp_kelvin = (beta * room_temp) / (beta + (room_temp * logf(R_therm / R_room_temp)));
  
  return temp_kelvin - 273.15;
}
