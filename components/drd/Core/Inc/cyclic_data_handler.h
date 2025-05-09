#ifndef CYCLIC_DATA_HANDLER_H
#define CYCLIC_DATA_HANDLER_H

// User Defines
#define MAX_CYCLE_TIME 3000 // Maximum cycle time in milliseconds

// Set functions for cyclic data
void set_cyclic_speed(uint32_t speed);
void set_cyclic_pack_current(int16_t current);
void set_cyclic_pack_voltage(uint16_t voltage);
void set_cyclic_drive_state(uint8_t state);
void set_cyclic_drive_mode(uint8_t mode);
void set_cyclic_soc(uint8_t soc);

// Get functions for cyclic data
uint32_t* get_cyclic_speed(void);
int16_t* get_cyclic_pack_current(void);
uint16_t* get_cyclic_pack_voltage(void);
uint8_t* get_cyclic_drive_state(void);
uint8_t* get_cyclic_drive_mode(void);
uint8_t* get_cyclic_soc(void);

#endif // CYCLIC_DATA_HANDLER_H
