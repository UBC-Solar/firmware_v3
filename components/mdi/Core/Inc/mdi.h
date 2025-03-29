/**
 * @file mdi.h
 * 
 * @brief Header file and user facing functions for the MDI
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MDI_H
#define __MDI_H

// Includes
#include <stdint.h>
#include <stdbool.h>

// Macros
#define MAX_DAC_VALUE (0.90 * 0x3FF)
#define MAX_TIMEOUT_VALUE (1000) // TODO: Is this time correct?
#define MDI_DIAGNOSTICS_DELAY (1000)

#define IS_BIT_SET(num, bit) (((num) & (1 << (bit))) != 0)


// Public typedefs
typedef enum {
	REGEN_DAC = (0b0001101 << 1),
    ACCEL_DAC = (0b0001110 << 1)
} MDI_DAC_addr_t;

typedef enum {
    FORWARD = 0,
    REVERSE = 1
} MDI_direction_t;

typedef enum {
    ECO_MODE   = 0,
    POWER_MODE = 1
} MDI_power_mode_t;

typedef struct{
    uint16_t accel_DAC_value;
    uint16_t regen_DAC_value;
    bool direction_value;
    bool eco_mode_value;
} MDI_motor_command_t;


// Public functions
void MDI_set_DAC_voltage(MDI_DAC_addr_t DAC_addr, uint16_t voltage_value);
void MDI_set_motor_command(MDI_motor_command_t* MDI_motor_command);
void MDI_parse_motor_command(uint8_t* buffer, MDI_motor_command_t* MDI_motor_command);
void MDI_stop_motor();

#endif
