/**
 *  @file analysis.c
 *  @brief Analysis functions to determine charge and temperature states of battery
 *
 *  @date 2020/07/10
 *  @author Andrew Hanlon (a2k-hanlon)
 */
#include "analysis.h"

// Function prototypes
float BTM_TEMP_volts2temp(uint16_t); // TODO: remove later; here until thermistor code is available
BTM_module_volt_state_t findModuleVoltState(uint16_t pack);
BTM_module_temp_state_t findModuleTempState(float temp);

/**
 * @brief Determine voltage-based state of all modules in the pack
 *
 * @param[in/out] pack Pointer to the pack data structure to update statuses in
 */
void ANA_analyzeModuleVoltages(BTM_PackData_t * pack) {
    uint16_t voltage = 0;

    for(int stack_num = 0; stack_num < BTM_NUM_DEVICES; stack_num++) {
        for(int module_num = 0; module_num < BTM_NUM_MODULES; module_num++) {
            voltage = pack->stack[stack_num].module[module_num].voltage;

            // If module is enabled...
            if (pack->stack[stack_num].module[module_num].enable)
            {
                // If the module has faulted (volt_state >= VOLTAGE_OV numeric value)
                // DO NOT update the volt state - this way faults persist
                if (pack->stack[stack_num].module[module_num].volt_state < VOLTAGE_OV)
                {
                    pack->stack[stack_num].module[module_num].volt_state = findModuleVoltState(voltage);
                }
            }
        } // end inner for loop
    } // end outer for loop

    return;
}

// Helper function for analyzeModuleVoltages()
BTM_module_volt_state_t findModuleVoltState(uint16_t voltage) {
    BTM_module_volt_state_t volt_state = VOLTAGE_NORMAL;

    if (voltage >= HLIM_FAULT_VOLTAGE)
    {
       volt_state = VOLTAGE_OV;
    }
    else if (voltage >= HLIM_WARNING_VOLTAGE)
    {
       volt_state = VOLTAGE_FULLCHARGE;
    }
    else if (voltage <= SHORT_FAULT_VOLTAGE)
    {
       volt_state = VOLTAGE_NONE;
    }
    else if (voltage <= LLIM_FAULT_VOLTAGE)
    {
       volt_state = VOLTAGE_UV;
    }
    else if (voltage <= LLIM_WARNING_VOLTAGE)
    {
       volt_state = VOLTAGE_LOWCHARGE;
    }

    // May return VOLTAGE_NORMAL - warnings are allowed to resolve,
    // faults are not (this is handled elsewhere)
    return volt_state;
}


/**
 * @brief Determine temp-based state of all modules in the pack
 *
 * @param[in/out] pack Pointer to the pack data structure to update statuses in
 */
void ANA_analyzeModuleTemps(BTM_PackData_t * pack) {
    float temp = 0.0;

    for(int stack_num = 0; stack_num < BTM_NUM_DEVICES; stack_num++) {
        for(int module_num = 0; module_num < BTM_NUM_MODULES; module_num++) {
            temp = BTM_TEMP_volts2temp(pack->stack[stack_num].module[module_num].temperature);

            // If module is enabled...
            if (pack->stack[stack_num].module[module_num].enable)
            {
                // If the module has faulted (temp_state >= TEMP_OT numeric value)
                // DO NOT update the temp state - this way faults persist
                if (pack->stack[stack_num].module[module_num].temp_state < TEMP_OT)
                {
                    pack->stack[stack_num].module[module_num].temp_state = findModuleTempState(temp);
                }
            }
        } // end inner for loop
    } // end outer for loop

    return;
}

// Helper function for analyzeModuleTemps()
BTM_module_temp_state_t findModuleTempState(float temp) {
    BTM_module_temp_state_t temp_state;

    if (temp >= HIGH_READ_LIMIT_TEMP)
    {
        temp_state = TEMP_BAD_READ;
    }
    else if (temp >= OT_FAULT_TEMP)
    {
        temp_state = TEMP_OT;
    }
    else if (temp >= HIGH_WARNING_TEMP)
    {
        temp_state = TEMP_HIGH;
    }
    else if (temp <= LOW_READ_LIMIT_TEMP)
    {
        temp_state = TEMP_BAD_READ;
    }
    else if (temp <= LOW_WARNING_TEMP)
    {
        temp_state = TEMP_LOW;
    }

    return temp_state;
}
