#include "util.h"
#include <stdio.h>

void UTIL_printModuleVoltages(Pack_t *pack){
    
    float module_voltage = 0;

    for (uint32_t module_num = 0; module_num < PACK_NUM_BATTERY_MODULES; module_num++)
    {
        module_voltage = ((float) pack->module[module_num].voltage) / 10000;
        printf("module %ld voltage: %.5f\r\n", module_num, module_voltage);
    }
}

void UTIL_printModuleTemps(Pack_t *pack){
    
    float module_temp = 0;

    for (uint32_t module_num = 0; module_num < PACK_NUM_BATTERY_MODULES; module_num++)
    {
        module_temp = pack->module[module_num].temperature;
        printf("module %ld temp: %.4f\r\n", module_num, module_temp);
    }
}