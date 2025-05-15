#include "cyclic_data.h"
#include "cyclic_data_handler.h"
#include "drive_state.h"
#include "lcd.h"

// CYCLIC_DATA(type, name, max_cycle_time)
CYCLIC_DATA(uint32_t, cyclic_speed, MAX_CYCLE_TIME);               // Vehicle speed (km/h)
CYCLIC_DATA(int16_t, cyclic_pack_current, MAX_CYCLE_TIME);      // Battery pack current
CYCLIC_DATA(uint16_t, cyclic_pack_voltage, MAX_CYCLE_TIME);     // Battery pack voltage
CYCLIC_DATA(uint8_t, cyclic_drive_state, MAX_CYCLE_TIME);       // Current drive state (e.g., PARK, FORWARD)
CYCLIC_DATA(uint8_t, cyclic_soc, MAX_CYCLE_TIME);               // State of Charge (SOC %)

// Create functions that update the cyclic data.
void set_cyclic_speed(uint32_t speed) {
   CYCLIC_DATA_SET(cyclic_speed, speed);
}

void set_cyclic_pack_current(int16_t current) {
   CYCLIC_DATA_SET(cyclic_pack_current, current);
}

void set_cyclic_pack_voltage(uint16_t voltage) {
   CYCLIC_DATA_SET(cyclic_pack_voltage, voltage);
}

void set_cyclic_drive_state(uint8_t state) {
   CYCLIC_DATA_SET(cyclic_drive_state, state);
}

void set_cyclic_soc(uint8_t soc) {
   CYCLIC_DATA_SET(cyclic_soc, soc);
}

// Create functions that get the cyclic_data

uint32_t* get_cyclic_speed(void) {
   return CYCLIC_DATA_GET(cyclic_speed);
}

int16_t* get_cyclic_pack_current(void) {
   return CYCLIC_DATA_GET(cyclic_pack_current);
}

uint16_t* get_cyclic_pack_voltage(void) {
   return CYCLIC_DATA_GET(cyclic_pack_voltage);
}

uint8_t* get_cyclic_drive_state(void) {
   if(get_cyclic_speed() == NULL){
      return NULL; // Stale data for drive state
   }
   else{
      return CYCLIC_DATA_GET(cyclic_drive_state);
   }
   
}

uint8_t* get_cyclic_soc(void) {
    if((get_cyclic_pack_voltage() == NULL) || (get_cyclic_pack_current() == NULL)){
        return NULL; 
    }
    else{
        return CYCLIC_DATA_GET(cyclic_soc);
    }
}
