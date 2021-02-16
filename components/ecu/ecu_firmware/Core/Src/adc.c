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
#define MAX_VOLT_READING 3.3
#define ADC_RESOLUTION 4095

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
int ADC_getSuppBattVoltage(unsigned int * supp_voltage)
{
    //initialize variables
    int volt_raw, i;
    float volt_adc = 0;
    int num_reads = 10;

    //reading a set of ADC voltages

    for (i = 0; i < num_reads; i ++) {
        
        //read raw value from ADC
        HAL_ADC_PollForConversion(ADC_supp_batt_volt, HAL_MAX_DELAY);
        volt_raw = HAL_GetValue(ADC_supp_batt_volt);

        //check if the ADC is saturated
        if (volt_raw >= )
            return 1;
        
        volt_adc += volt_raw;        
    }

    //convert ADC raw values to average voltage
    volt_adc = ((float) volt_raw / num_reads) * MAX_VOLT_READING / ADC_RESOLUTION;

    //assign the average ADC voltage to supp_voltage
    //adc voltage comes from a voltage divider made of R1=10k and R2=1k (v_adc = 1/11 * v_supp)

    *supp_voltage = volt_adc * 11 * 1000; 

    return 0;
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
int ADC_getMotorCurrent(int * motor_current)
{
    //initialize variables
    int volt_raw, num_reads;
    float volt_adc = 0;

    //read a set of ADC values for 1 second
    last_tick = HAL_GetTick();

    //interrupts
    //timer reset function
    //sets volt_adc back to 0 whenever
    while (!(HAL_GetTick() - last_tick == 1000)) {

        //read raw value from ADC
        HAL_ADC_PollForConversion(ADC_motor_current, HAL_MAX_DELAY);
        volt_raw = HAL_GetValue(ADC_motor_current);
        
        if (volt_raw >= )
            return 1;
        
        count ++;
        volt_adc += volt_raw;
    }

    //convert ADC raw values to average voltage
    volt_adc = ((float) volt_raw / num_reads) * MAX_VOLT_READING / ADC_RESOLUTION;

    //convert volt readings into current
    //good references: https://www.lem.com/sites/default/files/products_datasheets/hass_50_600-s.pdf 
    //file:///C:/Users/Forbes/Downloads/Isolated%20current%20and%20voltage%20transducers%20characteristics_1.pdf 

    *motor_current = (volt_adc - 2.5) * 80 * 100; //output in cA

    return 0;
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
int ADC_getArrayCurrent(int * array_current)
{
    //initialize variables
    int volt_raw, num_reads;
    float volt_adc = 0;

    //read a set of ADC values for 1 second
    last_tick = HAL_GetTick();
    while (!(HAL_GetTick() - last_tick == 1000)) {

        //read raw value from ADC
        HAL_ADC_PollForConversion(ADC_array_current, HAL_MAX_DELAY);
        volt_raw = HAL_GetValue(ADC_array_current);
        
        if (volt_raw >= )
            return 1;
        
        count ++;
        volt_adc += volt_raw;
    }

    //convert ADC raw values to average voltage
    volt_adc = ((float) volt_raw / num_reads) * MAX_VOLT_READING / ADC_RESOLUTION;

    //convert volt readings into current
    //good references: https://www.lem.com/sites/default/files/products_datasheets/hass_50_600-s.pdf 
    //file:///C:/Users/Forbes/Downloads/Isolated%20current%20and%20voltage%20transducers%20characteristics_1.pdf 

    *array_current = (volt_adc - 2.5) * 80 * 100; //output in cA

    return 0;
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
    return motor_current - array_current;
}

/*============================================================================*/
/* PRIVATE FUNCTIONS */
