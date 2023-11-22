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
  float adc_voltage = adc_reading;
  if (adc_voltage < 0) adc_voltage = 0;
  else if (adc_voltage >= ADC_RESOLUTION) adc_voltage = ADC_RESOLUTION;
  adc_voltage = adc_voltage * 1000.0 * ADC_MAX_VOLT_READING/ADC_RESOLUTION;
  float offset_ref = HASS_SENSOR_DEFAULT_VOLTAGE_OFFSET;

  switch (adc_channel)
  {
  case OFFSET_REF_AM__ADC1_IN0: //Records array and motor current sensor voltage offset in mV
    ADC_am_ref_offset = adc_voltage; 
    break;
  
  case LVS_CURR_SENSE__ADC1_IN4: //Records ECU low voltage current in cA
    ADC_lvs_current = (int)((adc_voltage - ACS781xLR_VOLTAGE_OFFSET * 1000) / ACS782xLR_CURRENT_SCALE_FACTOR * 100);
    break; 
  
  case SUPP_SENSE__ADC1_IN5: //Records supplementary battery voltage in mV
    ADC_supp_batt_volt = (int)(adc_voltage*SUPP_BATT_VOLTAGE_DIVIDER);
    break;

  case OFFSET_REF_BAT__ADC1_IN10: //Records battery current sensor voltage offset in mV
    ADC_batt_ref_offset = adc_voltage;
    break;

  case B_SENSE__ADC3_IN1: //Records battery current in cA
    offset_ref = (float)(ADC_getOffsetRef_Batt());
    ADC_battery_current = (int)(((adc_voltage - offset_ref)/1000.0) * HASS_100_S_CURRENT_SCALE_FACTOR * 100.0);
    break;

  case M_SENSE__ADC3_IN2: //Records motor current in cA
    offset_ref = (float)ADC_getOffsetRef_AM();
    ADC_motor_current = (int)(((adc_voltage - offset_ref)/1000.0) * HASS_50_600_S_CURRENT_SCALE_FACTOR * 100.0);
    break;
  
  case A_SENSE__ADC3_IN3: //Records array current in cA
    offset_ref = (float)ADC_getOffsetRef_AM();
    ADC_array_current = (int)(((adc_voltage - offset_ref)/1000.0) * HASS_50_600_S_CURRENT_SCALE_FACTOR * 100.0);
    break;

  default:
    break;
  }
}

/**
 * @brief Retrieves voltage offset of the motor and array current sensors
 * 
 */

float ADC_getOffsetRef_AM()
{
  return (float)ADC_am_ref_offset;
}

/**
 * @brief Retrieves voltage offset of the battery current sensor
 * 
 */
float ADC_getOffsetRef_Batt()
{
  return (float)ADC_batt_ref_offset;
}

/**
 * @brief Retrieves current input to the low voltage system
 * 
 */
int ADC_getLowVoltageCurrent()
{
  return ADC_lvs_current;
}

/**
 * @brief Retrieves battery current draw
 * 
 */
int ADC_getBatteryCurrent(){
  return ADC_battery_current;
}

/**
 * @brief Retrieves the voltage of the supplemental battery (mV)
 * 
 * The supplement battery voltage is stored in the global varialbe ADC3_supp_batt_volt (int datatype),
 * and the function returns that global variable
 * Supp battery only for startup not really in driving
 * 
 * @param -
 * @retval returns the global variable ADC3_supp_batt_volt (int datatype)
 */
int ADC_getSuppBattVoltage()
{
    return ADC_supp_batt_volt;
}

/**
 * @brief Retrieves the current flowing between the motor controller and battery pack (cA)
 * 
 * The motor current is stored in the global varialbe ADC3_motor_current (int datatype),
 * and the function returns that global variable
 * Current flowing **OUT** of the pack is positive.
 * 
 * @param -
 * @retval returns the global variable ADC3_motor_current (int datatype)
 */
int ADC_getMotorCurrent()
{
    return ADC_motor_current;
}

/**
 * @brief Retrieves the current flowing between the solar array and battery pack (cA)
 * 
 * The motor current is stored in the global varialbe ADC3_array_current (int datatype),
 * and the function returns that global variable
 * Current flowing **INTO** of the pack is positive.
 * 
 * @param -
 * @retval returns the global variable ADC3_array_current (int datatype)
 */
int ADC_getArrayCurrent()
{
    return ADC_array_current;
}


/**
 * @brief Calculates the net current flowing out of the battery
 * 
 * If the return value is positive, net current is flowing out of the
 * battery and it is discharging.
 * If the return value is negative, net current is flowing into the
 * battery and it is charging.
 * 
 * @param motor_current Current to motor (positive out of pack)
 * @param array_current Current from array (positive into pack)
 * @retval net current out of battery, in the same format as the inputs
 */
int ADC_netCurrentOut()
{
  return ADC_motor_current - ADC_array_current;
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
void ADC1_processRawReadings(int half, volatile uint32_t adc1_buf[], float result[])
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
 * @brief converts the raw readings of ADC3 into ADC voltage readings and
 * averages these values at half of its buffer size per channel, 
 * when the DMA interrupt for ADC3 is called when half of its scan 
 * conversion sequence is completed. These output values will then be converted
 * into supplemental battery voltage, current flowing between motor controller
 * and battery pack, and the current flowing between solar arrays and battery pack 
 * 
 * @param half integer (1 or 0) indicating if the averaging is from the first half (0)
 *             of its circular buffer or its second half (1)
 * @param adc3_buf volatile int array that represents ADC3's DMA circular buffer 
 *                 (200 elements per channel/measurement values; 600 in total. 
 *                  Each time the the ADC is read, the next 3 elements get populated 
 *                  with the battery current, motor current, and array current in 
 *                  that specific order)
 * @param result float array of size 3, storing the averaged ADC voltages (float) for
 *               battery current, motor current, and array current
 * @retval stores the averaged readings of ADC3 to result[]
 */
void ADC3_processRawReadings(int half, volatile uint32_t adc3_buf[], float result[])
{
  int32_t sum[ADC3_NUM_ANALOG_CHANNELS] = {0}; //used for summing in the averaging process
  
  int sample_num = 0; //start index for averaging the ADC readings
  int limit = ADC3_BUF_LENGTH_PER_CHANNEL >> 1; // divide by 2; the end index of the averaging process

  if(half == 1) //start and end indices if averaging the second half of the ADC3's DMA circular buffer
  {
    sample_num = ADC3_BUF_LENGTH_PER_CHANNEL >> 1; 
    limit = ADC3_BUF_LENGTH_PER_CHANNEL;
  }

  //for (int n =0; n < 600; n++) printf("%d\n", adc3_buf[n]);
  //printf("%d || %d\n", sample_num, limit);
  // Average the samples
  for(; sample_num < limit; sample_num++) //summing the readings in the averaging process
  {
    
    //printf("%f\n", adc3_buf[sample_num]);
    for(int channel = 0; channel < ADC3_NUM_ANALOG_CHANNELS; channel++)
    {
      //adc3_buf is organized as [supp batt x 200 ... motor curr x 200 ... array curr x 200]
      //when sampling at 100Hz
      sum[channel] += (float)adc3_buf[ADC3_NUM_ANALOG_CHANNELS * sample_num + channel];
    }
    //printf("%d\n", adc3_buf[sample_num]); 
  }
  
  //the division process of the averaging
  for(int channel = 0; channel < ADC3_NUM_ANALOG_CHANNELS; channel++)
  {
    //averages for supplemental batery voltage, motor current, and array current
    result[channel] = ((float) sum[channel]) / (ADC3_BUF_LENGTH_PER_CHANNEL >> 1);
  }
}


/**
 * @brief sets the busy flag in the global variable ADC3_DMA_in_process_flag
 * depending if ADC3 attempts to read values in the middle of a DMA interrupt callback process
 * 
 * @param flag_value integer value: 1 is busy, 0 is not at busy
 * @retval sets ADC3_DMA_in_process_flag with flag_value
 */
void ADC3_setBusyStatus(int flag_value)
{
  ADC3_DMA_in_process_flag = flag_value;
}

/**
 * @brief Retrieves the busy status of ADC3, stored in global variable
 * ADC3_DMA_in_process_flag
 * 
 * @param -
 * @retval returns the global variable ADC3_DMA_in_process_flag (int datatype).
 *         1 means at busy; 0 means not at busy
 */
int ADC3_getBusyStatus() 
{
  return ADC3_DMA_in_process_flag;
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


/**
 * @brief sets the Fault flag in the global variable ADC1_DMA_fault_flag
 * depending if ADC1 attempts to read values in the middle of a DMA interrupt callback process
 * 
 * @param flag_value integer value: 1 is at fault, 0 is not at fault
 * @retval sets ADC1_DMA_fault_flag with flag_value
 */
void ADC3_setFaultStatus(int flag_value)
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
int ADC3_getFaultStatus() 
{
  return ADC1_DMA_fault_flag;
}

/*============================================================================*/
/* PRIVATE FUNCTIONS */