#ifndef CYCLIC_DATA_HANDLER_H
#define CYCLIC_DATA_HANDLER_H

// Set functions for cyclic data
void set_cyclic_speed(float speed);
void set_cyclic_pack_current(uint16_t current);
void set_cyclic_pack_voltage(uint16_t voltage);
void set_cyclic_drive_state(uint8_t state);
void set_cyclic_drive_mode(uint8_t mode);
void set_cyclic_soc(uint8_t soc);

// Get functions for cyclic data
float    get_cyclic_speed(void);
uint16_t get_cyclic_pack_current(void);
uint16_t get_cyclic_pack_voltage(void);
uint8_t  get_cyclic_drive_state(void);
uint8_t  get_cyclic_drive_mode(void);
uint8_t  get_cyclic_soc(void);

#endif // CYCLIC_DATA_HANDLER_H