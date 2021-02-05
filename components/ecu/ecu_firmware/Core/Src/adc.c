/**
 * @file adc.c
 * @brief ADC interface for supplemental battery and current sensor read on ECU
 * 
 * These functions need to average out the ADC readings for accuracy.
 * 
 * @date 2021/01/30
 * @author 
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
 * The voltage is averaged over XXX seconds OR XXX readings.
 * 
 * @param supp_voltage Pointer to variable to which result will be written
 * @retval 0 if reading is successful, else 1 (due to eg. ADC saturated)
 */
int ADC_getSuppBattVoltage(unsigned int * supp_voltage)
{
    return 0;
}

/**
 * @brief Provides current flowing between the motor controller and battery pack
 * 
 * Current is averaged over 1 second. The result is provided as a
 * signed integer with units of 100ths of an amp (cA).
 * Current flowing **OUT** of the pack is positive.
 * 
 * @param motor_current Pointer to variable to which result will be written
 * @retval 0 if reading is successful, else 1 (due to eg. ADC saturated)
 */
int ADC_getMotorCurrent(int * motor_current)
{
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
