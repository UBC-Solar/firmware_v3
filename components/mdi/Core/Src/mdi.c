/**
 * @file mdi.c
 * 
 * @brief This file contains helper functions to control the mitsuba motor controller.
 * The MDI receives motor command messages from the DRD board via CAN and controls the Mitsuba MDU via the accel and regen DACs.
 * 
 * @attention See datasheets for the DRD and Mitsuba for full breakdown
 */


#include <stdbool.h>
#include "mdi.h"
#include "i2c.h"
#include "gpio.h"


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

    // TODO: Replace blocking call
    HAL_I2C_Master_Transmit(&hi2c2, DAC_addr, i2c_buffer, sizeof(i2c_buffer), HAL_MAX_DELAY);
}


/**
 * @brief Sets the DAC voltages based on the global accel and regen DAC values
 * @note Must call MDI_parse_motor_command() to set the global accel and regen DAC values
 */
void MDI_set_motor_command(MDI_motor_command_t* MDI_motor_command)
{
    MDI_set_DAC_voltage(ACCEL_DAC, MDI_motor_command->accel_DAC_value);
    MDI_set_DAC_voltage(REGEN_DAC, MDI_motor_command->regen_DAC_value);

    HAL_GPIO_WritePin(DIR_GPIO_Port, DIR_Pin, MDI_motor_command->direction_value);
    HAL_GPIO_WritePin(DIR_GPIO_Port, DIR_Pin, MDI_motor_command->eco_mode_value);
}


/**
 * @brief Sets the global accel and regen DAC values based on the input buffer
 * 
 * @param buffer Input CAN buffer
 * 
 * @note This function is designed to be called in the CAN Rx ISR and only parses and updates the global variables.
 * Call MDI_set_motor_command() after ISR to set the DAC values
 */
void MDI_parse_motor_command(uint8_t* buffer, MDI_motor_command_t* MDI_motor_command)
{
    // Reset to 0
    MDI_motor_command->accel_DAC_value = 0;
    MDI_motor_command->regen_DAC_value = 0;

    // Populate accel and regen DAC values
    MDI_motor_command->accel_DAC_value |= buffer[0];
    MDI_motor_command->accel_DAC_value |= buffer[1] << 8;

    MDI_motor_command->regen_DAC_value |= buffer[2];
    MDI_motor_command->regen_DAC_value |= buffer[3] << 8;

    // Set direction and eco mode values
    MDI_motor_command->direction_value = IS_BIT_SET(buffer[4], 0);
    MDI_motor_command->eco_mode_value = IS_BIT_SET(buffer[4], 1);
}

/**
 * @brief Sets the inputs to the MDU to the default values for safety.
 * Both DACs are set to 0. Direction is forward and eco mode is turned on
 * 
 */
void MDI_stop()
{
    MDI_motor_command_t MDI_motor_command;
    MDI_motor_command.accel_DAC_value = 0;
    MDI_motor_command.regen_DAC_value = 0;
    MDI_motor_command.direction_value = 1;
    MDI_motor_command.eco_mode_value = 0;

    MDI_set_motor_command(&MDI_motor_command);
}
