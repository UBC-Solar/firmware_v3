/**
 * @file mdi.h
 * 
 * @brief Header file and user facing functions for the MDI
 */

// Includes
#include <stdint.h>

// Macros
#define MAX_DAC_VALUE (0.90 * 0x3FF)
#define DRD_MOTOR_COMMAND_CAN_ID (0x400)
#define MAX_TIMEOUT_VALUE (1000) // TODO: Is this time correct?
#define IS_BIT_SET(num, bit) (((num) & (1 << (bit))) != 0)


// Public typedefs
typedef enum {
	REGEN_DAC = (0b0001101 << 1),
    ACCEL_DAC = (0b0001110 << 1)
} motor_DAC_addr_t;

typedef struct{
    uint16_t accel_DAC_value;
    uint16_t regen_DAC_value;
    bool direction_value;
    bool eco_mode_value;
} MDI_motor_command_t;


// Public functions
void MDI_set_DAC_voltage(motor_DAC_addr_t DAC_addr, uint16_t voltage_value);
void MDI_set_motor_command(MDI_motor_command_t* MDI_motor_command);
void MDI_parse_motor_command(uint8_t* buffer, MDI_motor_command_t* MDI_motor_command);
void MDI_stop();
