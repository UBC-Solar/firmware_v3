/**
 * @file mdi.c
 * 
 * @brief This file contains helper functions to control the mitsuba motor controller.
 * The MDI receives motor command messages from the DRD board via CAN and controls the Mitsuba MDU via the accel and regen DACs.
 * 
 * @attention See datasheets for the DRD and Mitsuba for full breakdown
 */

#include "mdi.h"
#include "i2c.h"

// Private Variables
uint16_t g_accel_DAC_value = 0;
uint16_t g_regen_DAC_value = 0;

/**
 * @brief Sets the voltage of an individual DAC
 * 
 * @param DAC_addr enum address of the DAC. Either the accel or regen DAC
 * @param voltage_value 10 bit unsigned integer. Scaled linearly to the voltage value
 */
void MDI_set_DAC_voltage(motor_DAC_addr_t DAC_addr, uint16_t voltage_value)
{
    // Truncate if voltage_value is greater than max
    voltage_value = (voltage_value > MAX_DAC_VALUE) ? MAX_DAC_VALUE : voltage_value;

    uint8_t i2c_buffer[2];

    i2c_buffer[0] = voltage_value >> 8;
    i2c_buffer[1] = voltage_value;

    // TODO: Replace blocking call HAL_MAX_DELAY
    HAL_I2C_Master_Transmit(&hi2c2, DAC_addr, i2c_buffer, sizeof(i2c_buffer), HAL_MAX_DELAY);
}


/**
 * @brief Sets the DAC voltages based on the global accel and regen DAC values
 * @note Must call MDI_parse_motor_command() to set the global accel and regen DAC values
 */
void MDI_set_DACs()
{
    MDI_set_DAC_voltage(ACCEL_DAC, g_accel_DAC_value);
    MDI_set_DAC_voltage(REGEN_DAC, g_regen_DAC_value);
}

/**
 * @brief Sets the global accel and regen DAC values based on the input buffer
 * 
 * @param buffer Input CAN buffer
 * 
 * @note This function is designed to be called in the CAN Rx ISR and only parses and updates the global variables.
 * Call MDI_set_DACs() after ISR to set the DAC values
 */
void MDI_parse_motor_command(uint8_t* buffer)
{
    g_accel_DAC_value = 0;
    g_regen_DAC_value = 0;

    g_accel_DAC_value |= buffer[0];
    g_accel_DAC_value |= buffer[1] << 8;

    g_regen_DAC_value |= buffer[2];
    g_regen_DAC_value |= buffer[3] << 8;
}
