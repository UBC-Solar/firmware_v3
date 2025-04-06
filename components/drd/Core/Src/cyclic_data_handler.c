#include "cyclic_data.h"
#include "cyclic_data_handler.h"
#include "drive_state.h"
#include "lcd.h"

// CYCLIC_DATA(type, name, max_cycle_time)
CYCLIC_DATA(float, cyclic_speed, MAX_CYCLE_TIME);               // Vehicle speed (km/h)
CYCLIC_DATA(int16_t, cyclic_pack_current, MAX_CYCLE_TIME);      // Battery pack current
CYCLIC_DATA(uint16_t, cyclic_pack_voltage, MAX_CYCLE_TIME);     // Battery pack voltage
CYCLIC_DATA(uint8_t, cyclic_drive_state, MAX_CYCLE_TIME);       // Current drive state (e.g., PARK, FORWARD)
CYCLIC_DATA(uint8_t, cyclic_drive_mode, MAX_CYCLE_TIME);        // Drive mode (e.g., ECO or POWER)
CYCLIC_DATA(uint8_t, cyclic_soc, MAX_CYCLE_TIME);               // State of Charge (SOC %)

// Create functions that update the cyclic data.
void set_cyclic_speed(float speed) {
   CYCLIC_DATA_SET(cyclic_speed, speed);
}

void set_cyclic_pack_current(uint16_t current) {
   CYCLIC_DATA_SET(cyclic_pack_current, current);
}

void set_cyclic_pack_voltage(uint16_t voltage) {
   CYCLIC_DATA_SET(cyclic_pack_voltage, voltage);
}

void set_cyclic_drive_state(uint8_t state) {
   CYCLIC_DATA_SET(cyclic_drive_state, state);
}

void set_cyclic_drive_mode(uint8_t mode) {
   CYCLIC_DATA_SET(cyclic_drive_mode, mode);
}

void set_cyclic_soc(uint8_t soc) {
   CYCLIC_DATA_SET(cyclic_soc, soc);
}

// Create functions that get the cyclic_data

float get_cyclic_speed(void) {
   float* cyclic_speed_data = CYCLIC_DATA_GET(cyclic_speed);
   if (cyclic_speed_data == NULL) {
       return -1.0f;
   }
   return *cyclic_speed_data;
}

uint16_t get_cyclic_pack_current(void) {
   int16_t* cyclic_pack_current_data = CYCLIC_DATA_GET(cyclic_pack_current);
   if (cyclic_pack_current_data == NULL) {
       return 0xFFFF;
   }
   return *cyclic_pack_current_data;
}

uint16_t get_cyclic_pack_voltage(void) {
   uint16_t* cyclic_pack_voltage_data = CYCLIC_DATA_GET(cyclic_pack_voltage);
   if (cyclic_pack_voltage_data == NULL) {
       return 0xFFFF;
   }
   return *cyclic_pack_voltage_data;
}

uint8_t get_cyclic_drive_state(void) {
   uint8_t* cyclic_drive_state_data = CYCLIC_DATA_GET(cyclic_drive_state);
   if (cyclic_drive_state_data == NULL) {
       return 0xFF;
   }
   return *cyclic_drive_state_data;
}

uint8_t get_cyclic_drive_mode(void) {
   uint8_t* cyclic_drive_mode_data = CYCLIC_DATA_GET(cyclic_drive_mode);
   if (cyclic_drive_mode_data == NULL) {
       return 0xFF;
   }
   return *cyclic_drive_mode_data;
}

uint8_t get_cyclic_soc(void) {
   uint8_t* cyclic_soc_data = CYCLIC_DATA_GET(cyclic_soc);
   if (cyclic_soc_data == NULL) {
       return 0xFF;
   }
   return *cyclic_soc_data;
}
