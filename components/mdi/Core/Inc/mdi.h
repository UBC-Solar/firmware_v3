/**
 * @file mdi.h
 * 
 * @brief Header file and user facing functions for the MDI
 */


#include <stdint.h>


#define MAX_DAC_VALUE (0.90 * 0x3FF)
#define DRD_MOTOR_COMMAND_CAN_ID (0x400)
#define MAX_TIMEOUT_VALUE (1000) // TODO: Is this time correct?


typedef enum {
	REGEN_DAC = (0b0001101 << 1),
    ACCEL_DAC = (0b0001110 << 1)
} motor_DAC_addr_t;


// Public functions
void MDI_set_DAC_voltage(motor_DAC_addr_t DAC_addr, uint16_t voltage_value);
void MDI_set_DACs();
void MDI_parse_motor_command(uint8_t* buffer);