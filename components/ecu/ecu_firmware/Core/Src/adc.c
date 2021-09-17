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
// #define MAX_VOLT_READING 3.3
// #define ADC_RESOLUTION 4095
// #define ADC_AVG_SIZE 10

/*============================================================================*/
/* PRIVATE FUNCTION PROTOTYPES */
// int ADC_getAverage (volatile int buffer[]);

/*============================================================================*/
/* PUBLIC FUNCTIONS */

// void ADC_init(void)
// {
//     ADC_buffer_index = 0;
//     ADC_raw_average = 0;
//     return;
// }

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
    ADC3_supp_batt_volt = supp_voltage * ADC_MAX_VOLT_READING / ADC_RESOLUTION * 11 * 1000; 
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
int ADC3_setMotorCurrent(float motor_current)
{
    //convert volt readings into current
    //good references: https://www.lem.com/sites/default/files/products_datasheets/hass_50_600-s.pdf 
    //file:///C:/Users/Forbes/Downloads/Isolated%20current%20and%20voltage%20transducers%20characteristics_1.pdf 

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
int ADC3_setArrayCurrent(float array_current)
{
    //initialize variables
    array_current = array_current * ADC_MAX_VOLT_READING / ADC_RESOLUTION;
    
    //convert volt readings into current
    //good references: https://www.lem.com/sites/default/files/products_datasheets/hass_50_600-s.pdf 
    //file:///C:/Users/Forbes/Downloads/Isolated%20current%20and%20voltage%20transducers%20characteristics_1.pdf 

    ADC3_array_current = (array_current - 2.5) * 80 * 100; //output in cA

    return 0;
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

void ADC3_processRawReadings(int half, float result[])
{
  int32_t sum[NUM_ANALOG_CHANNELS] = {0};
  int sample_num = 0;
  int limit = ADC_BUF_LENGTH_PER_CHANNEL >> 1; // divide by 2
  if(half == 1) 
  {
    sample_num = ADC_BUF_LENGTH_PER_CHANNEL >> 1; 
    limit = ADC_BUF_LENGTH_PER_CHANNEL;
  }

  // Average the samples
  for(; sample_num < limit; sample_num++)
  {
    for(int channel = 0; channel < NUM_ANALOG_CHANNELS; channel++)
    {
      sum[channel] += adc_buf[NUM_ANALOG_CHANNELS * sample_num + channel];
    }
  }
  
  for(int channel = 0; channel < NUM_ANALOG_CHANNELS; channel++)
  {
    result[channel] = ((float) sum[channel]) / (ADC_BUF_LENGTH_PER_CHANNEL >> 1);
  }

  return;
}

// void ADC_supp_batt_volt_runInterrupt(void)
// {

//   HAL_ADC_PollForConversion(ADC_supp_batt_volt, HAL_MAX_DELAY);
//   int volt_raw = HAL_ADC_GetValue(ADC_supp_batt_volt);

//   ADC_supp_batt_volt_buff[ADC_buffer_index] = volt_raw;
//   ADC_buffer_index++;

//   if (ADC_buffer_index >= ADC_BUFFER_SIZE - 1) {
//     ADC_buffer_index = 0;
//   }
// }

// void ADC_motor_current_runInterrupt(void)
// {

//   HAL_ADC_PollForConversion(ADC_motor_current, HAL_MAX_DELAY);
//   int volt_raw = HAL_ADC_GetValue(ADC_motor_current);

//   ADC_motor_current_buff[ADC_buffer_index] = volt_raw;
//   ADC_buffer_index++;

//   if (ADC_buffer_index >= ADC_BUFFER_SIZE - 1) {
//     ADC_buffer_index = 0;
//   }
// }

// void ADC_array_current_runInterrupt(void)
// {

//   HAL_ADC_PollForConversion(ADC_array_current, HAL_MAX_DELAY);
//   int volt_raw = HAL_ADC_GetValue(ADC_array_current);

//   ADC_array_current_buff[ADC_buffer_index] = volt_raw;
//   ADC_buffer_index ++;

//   if (ADC_buffer_index >= ADC_BUFFER_SIZE) {
//     ADC_buffer_index = 0;
//   }
// }

/*============================================================================*/
/* PRIVATE FUNCTIONS */
// int ADC_getAverage(volatile int buffer[])
// {
//     int i = 0;
//     int avg_index = ADC_buffer_index - 1;

//     for (i = 0; i < ADC_AVG_SIZE; i++) {

//         if (buffer[avg_index] <= ADC_RESOLUTION) 
//             ADC_raw_average += buffer[avg_index];
//         else 
//             return 1;

//         if (avg_index != 0) //read the most recent values
//             avg_index --;
//         else 
//             avg_index = ADC_BUFFER_SIZE - 1; //wrapping in the circular buffer
//     }

//     ADC_raw_average /= ADC_AVG_SIZE;
    
//     return 0;
// }
