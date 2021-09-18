/**
 * @file adc.c
 * @brief ADC interface for supplemental battery and current sensor read on ECU
 * 
 * These functions need to average out the ADC readings for accuracy.
 * 
 * @date 2021/01/30
 * @author Forbes Choy 
 */

#include "adc.h"

/*============================================================================*/
/* PRIVATE FUNCTION PROTOTYPES */

/*============================================================================*/
/* PUBLIC FUNCTIONS */

/**
 * @brief Provides the voltage of the supplemental Battery
 * 
 * The supplemental battery voltage is provided as an unsigned integer
 * with units of millivolts (mV).
 * The voltage is averaged over XXX seconds OR XXX readings. Cache of like 10 ADCs? Take the average and we can use the "immediate" value for "checking" (fault state)
 * ADC function could return the average and instant value (we can take in two pointers and overwrite them)
 * Expect several calls in one second
 * Supp battery only for startup not really in driving
 * 
 * @param supp_voltage Pointer to variable to which result will be written
 * @retval 0 if reading is successful, else 1 (due to eg. ADC saturated)
 */
void ADC3_setSuppBattVoltage(float supp_voltage)
{
    //assign the average ADC voltage to supp_voltage
    //adc voltage comes from a voltage divider made of R1=10k and R2=1k (v_adc = 1/11 * v_supp)
    ADC3_supp_batt_volt = (supp_voltage * ADC_MAX_VOLT_READING / ADC_RESOLUTION) * 11 * 1000; 
}

int ADC3_getSuppBattVoltage()
{
    return ADC3_supp_batt_volt;
}

/**
 * @brief Provides current flowing between the motor controller and battery pack
 * 
 * Current is averaged over 1 second. The result is provided as a
 * signed integer with units of 100ths of an amp (cA).
 * 
 * Current flowing **OUT** of the pack is positive.
 * Hardware exists to detect overcurrent
 * Output pins are wired to comparators to a set voltage
 * 
 * @param motor_current Pointer to variable to which result will be written
 * @retval 0 if reading is successful, else 1 (due to eg. ADC saturated)
 */
void ADC3_setMotorCurrent(float motor_current)
{
    //convert volt readings into current
    //good references: https://www.lem.com/sites/default/files/products_datasheets/hass_50_600-s.pdf 

    motor_current = motor_current * ADC_MAX_VOLT_READING / ADC_RESOLUTION;

    ADC3_motor_current = (motor_current - 2.5) * 80 * 100; //output in cA

    return 0;
}

int ADC3_getMotorCurrent()
{
    return ADC3_motor_current;
}

/**
 * @brief Provides current flowing between the solar array and battery pack
 * 
 * Current is averaged over 1 second. The result is provided as a
 * signed integer with units of 100ths of an amp (cA).
 * Current flowing **INTO** the pack is positive.
 * 
 * @param array_current Pointer to variable to which result will be written
 * @retval 0 if reading is successful, else 1 (due to eg. ADC saturated)
 */
void ADC3_setArrayCurrent(float array_current)
{
    //convert raw readings to voltage readings
    array_current = array_current * ADC_MAX_VOLT_READING / ADC_RESOLUTION;
    
    //convert volt readings into current
    //good references: https://www.lem.com/sites/default/files/products_datasheets/hass_50_600-s.pdf 

    ADC3_array_current = (array_current - 2.5) * 80 * 100; //output in cA
}

int ADC3_getArrayCurrent()
{
    return ADC3_array_current;
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
int ADC_netCurrentOut(int motor_current, int array_current)
{
    return ADC3_motor_current - ADC3_array_current;
}

void ADC3_processRawReadings(int half, volatile int adc3_buf[], float result[])
{
  int sum[ADC3_NUM_ANALOG_CHANNELS] = {0}; //C++(20) does not recognize int32_t
  int sample_num = 0;
  int limit = ADC3_BUF_LENGTH_PER_CHANNEL >> 1; // divide by 2
  if(half == 1) 
  {
    sample_num = ADC3_BUF_LENGTH_PER_CHANNEL >> 1; 
    limit = ADC3_BUF_LENGTH_PER_CHANNEL;
  }

  // Average the samples
  for(; sample_num < limit; sample_num++)
  {
    for(int channel = 0; channel < ADC3_NUM_ANALOG_CHANNELS; channel++)
    {
      sum[channel] += adc3_buf[ADC3_NUM_ANALOG_CHANNELS * sample_num + channel];
    }
  }
  
  for(int channel = 0; channel < ADC3_NUM_ANALOG_CHANNELS; channel++)
  {
    result[channel] = ((float) sum[channel]) / (ADC3_BUF_LENGTH_PER_CHANNEL >> 1);
  }
}

/*============================================================================*/
/* PRIVATE FUNCTIONS */