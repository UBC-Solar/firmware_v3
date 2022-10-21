/**
 *  @file analysis.c
 *  @brief Analysis functions to determine charge and temperature states of battery
 *
 *  @date 2020/07/10
 *  @author Andrew Hanlon (a2k-hanlon)
 */
#include "analysis.h"

// Private function prototypes
int findModuleVoltState(int status, uint16_t pack);
int findModuleTempState(int status, float temp);

/**
 * @brief Determines voltage- and temperature-based state of each module in the pack
 *
 * @param[in/out] pack Pointer to the pack data structure to update module statuses in
 */
void ANA_analyzeModules(BTM_PackData_t *pack)
{
    uint16_t voltage = 0;
    float temp = 0.0;
    int module_status = 0;
    struct BTM_module *module_p; // pointer to module being analyzed

    for (int stack_num = 0; stack_num < BTM_NUM_DEVICES; stack_num++)
    {
        for (int module_num = 0; module_num < BTM_NUM_MODULES; module_num++)
        {
            module_p = &(pack->stack[stack_num].module[module_num]);

            // If module is enabled...
            if (module_p->enable)
            {
                // Get key readings
                voltage = module_p->voltage;
                temp = module_p->temperature;
                // Get existing status
                module_status = module_p->status;
                // Update module status
                module_status = findModuleVoltState(module_status, voltage);
                module_status = findModuleTempState(module_status, temp);
                module_p->status = module_status;
            }
        } // end inner for loop
    }     // end outer for loop

    //

    return;
}

// Helper function for ANA_analyzeModules()
int findModuleVoltState(int status, uint16_t voltage)
{
    // Some conditionals here have no else because faults don't clear

    if (voltage >= OV_FAULT_VOLTAGE)
    {
        status |= BMS_FAULT_OV; // Set FAULT_OV bit
    }

    if (voltage >= HLIM_TRIP_VOLTAGE)
    {
        status |= BMS_TRIP_HLIM; // Set TRIP_HLIM bit
    }
    else
    {
        status &= ~BMS_TRIP_HLIM; // Clear TRIP_HLIM bit
    }

    if (voltage >= HIGH_WARNING_VOLTAGE)
    {
        status |= BMS_WARNING_HIGH_V; // Set WARNING_HIGH_V bit
    }
    else
    {
        status &= ~BMS_WARNING_HIGH_V; // Clear WARNING_HIGH_V bit
    }

    if (voltage <= SHORT_FAULT_VOLTAGE)
    {
        status |= BMS_FAULT_NO_VOLT; // Set FAULT_NO_VOLT bit
    }

    if (voltage <= UV_FAULT_VOLTAGE)
    {
        status |= BMS_FAULT_UV; // Set BMS_FAULT_UV
    }

    if (voltage <= LLIM_TRIP_VOLTAGE)
    {
        status |= BMS_TRIP_LLIM; // Set TRIP_LLIM bit
    }
    else
    {
        status &= ~BMS_TRIP_LLIM; // Clear TRIP_LLIM bit
    }

    if (voltage <= LOW_WARNING_VOLTAGE)
    {
        status |= BMS_WARNING_LOW_V; // Set WARNING_LOW_V bit
    }
    else
    {
        status &= ~BMS_WARNING_LOW_V; // Clear WARNING_LOW_V bit
    }

    return status;
}

// Helper function for ANA_analyzeModules()
int findModuleTempState(int status, float temp)
{
    // Some conditionals here have no else because faults don't clear

    if (temp >= HIGH_READ_LIMIT_TEMP || temp <= LOW_READ_LIMIT_TEMP)
    {
        status |= BMS_FAULT_TEMP_RANGE; // Set FAULT_TEMP_RANGE bit
    }

    if (temp >= OT_FAULT_TEMP)
    {
        status |= BMS_FAULT_OT; // Set FAULT_OT bit
    }

    if (temp >= CHARGE_OT_TEMP)
    {
        status |= BMS_TRIP_CHARGE_OT; // Set TRIP_CHARGE_OT bit
    }
    else
    {
        status &= ~BMS_TRIP_CHARGE_OT; // Clear TRIP_CHARGE_OT bit
    }

    if (temp >= HIGH_WARNING_TEMP)
    {
        status |= BMS_WARNING_HIGH_T; // Set WARNING_HIGH_T bit
    }
    else
    {
        status &= ~BMS_WARNING_HIGH_T; // Clear WARNING_HIGH_T bit
    }

    if (temp <= LOW_WARNING_TEMP)
    {
        status |= BMS_WARNING_LOW_T; // Set WARNING_LOW_T bit
    }
    else
    {
        status &= ~BMS_WARNING_LOW_T; // Clear WARNING_LOW_T bit
    }

    return status;
}

/**
 * @brief Generates a single BMS status code as an accumulation of all enabled modules' statuses
 *
 * @param[in] pack Pack data structure to read module statuses from
 * @return status code as a (32-bit) integer
 */
int ANA_mergeModuleStatusCodes(BTM_PackData_t *pack)
{
    int status_result = 0; // Start with a clean code
    struct BTM_module *module_p;

    // Just OR together status codes from all enabled modules
    for (int stack_num = 0; stack_num < BTM_NUM_DEVICES; stack_num++)
    {
        for (int module_num = 0; module_num < BTM_NUM_MODULES; module_num++)
        {
            module_p = &(pack->stack[stack_num].module[module_num]);
            if (module_p->enable)
            {
                status_result |= module_p->status;
            }
        }
    }

    return status_result;
}

/**
 * @brief Finds the value of the highest module temperature in the battery pack
 *
 * @param pack Data pack to search for highest module temperature
 * @return Floating point value in degrees C of the hottest module's temperature
 */
float ANA_findHighestModuleTemp(BTM_PackData_t *pack)
{
    float temperature = 0;
    float max_temperature = 0;
    struct BTM_module *module_p;

    for (int stack_num = 0; stack_num < BTM_NUM_DEVICES; stack_num++)
    {
        for (int module_num = 0; module_num < BTM_NUM_MODULES; module_num++)
        {
            module_p = &(pack->stack[stack_num].module[module_num]);

            // If module is enabled...
            if (module_p->enable)
            {
                temperature = module_p->temperature;
                if (temperature > max_temperature)
                    max_temperature = temperature;
            }
        }
    }

    return max_temperature;
}
