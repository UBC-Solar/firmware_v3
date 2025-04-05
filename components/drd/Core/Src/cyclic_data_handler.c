// cyclic_data_handler.c
#include "cyclic_data.h"
#include "cyclic_data_handler.h"
#include "drive_state.h"
#include "lcd.h"

//Global variables

// Create cyclic data structs
/* speed - g_velocity_kmh
** pack_current
** pack_voltage
** drive_state
** drive_mode - g_eco_mode
** soc
** drive_mode
*/
/*	Datatypes */
CYCLIC_DATA(float, cyclic_speed, 3000);  
CYCLIC_DATA(int16_t, cyclic_pack_current, 3000);
CYCLIC_DATA(uint16_t, cyclic_pack_voltage, 3000);
CYCLIC_DATA(uint8_t, cyclic_drive_state, 3000);  
CYCLIC_DATA(uint8_t, cyclic_drive_mode, 3000);
CYCLIC_DATA(uint8_t, cyclic_soc, 3000);

// Create functions that update the cyclic data.
void set_cyclic_speed(float speed){
   CYCLIC_DATA_SET(cyclic_speed, speed);
}

void set_cyclic_pack_current (uint16_t current){
   CYCLIC_DATA_SET(cyclic_pack_current, current);
}

void set_cyclic_pack_voltage (uint16_t voltage){
   CYCLIC_DATA_SET(cyclic_pack_voltage, voltage);
}

void set_cyclic_drive_state(uint8_t state){
   CYCLIC_DATA_SET(cyclic_drive_state, state);
}

void set_cyclic_drive_mode(uint8_t mode){
   CYCLIC_DATA_SET(cyclic_drive_mode, mode);
}

void set_cyclic_soc(uint8_t soc){
   CYCLIC_DATA_SET(cyclic_soc, soc);
}

// Create functions that get the cyclic_data
float get_cyclic_speed(void){
   float* cyclic_speed_data = CYCLIC_DATA_GET(cyclic_speed);
   return *cyclic_speed_data;
}

uint16_t get_cyclic_pack_current(void){
   int16_t* cyclic_pack_current_data = CYCLIC_DATA_GET(cyclic_pack_current);
   return *cyclic_pack_current_data;
}

uint16_t get_cyclic_pack_voltage(void){
   int16_t* cyclic_pack_voltage_data = CYCLIC_DATA_GET(cyclic_pack_voltage);
   return *cyclic_pack_voltage_data;
}

uint8_t get_cyclic_drive_state(void){
   uint8_t* cyclic_drive_state_data = CYCLIC_DATA_GET(cyclic_drive_state);
   return *cyclic_drive_state_data;
}

uint8_t get_cyclic_drive_mode(void){
   uint8_t* cyclic_drive_mode_data = CYCLIC_DATA_GET(cyclic_drive_mode);
   return *cyclic_drive_mode_data;
}

uint8_t get_cyclic_soc(void){
   uint8_t* cyclic_soc_data = CYCLIC_DATA_GET(cyclic_soc);
   return *cyclic_soc_data;
}
