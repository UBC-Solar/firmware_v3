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
        }
    }
    return;
}

// Helper function for ANA_analyzeModules()
int findModuleVoltState(int status, uint16_t voltage)
{
    // Some conditionals here have no else because faults don't clear

    // Faults

    if (voltage <= FLT_UV_THRESHOLD)
    {
        status |= FLT_UV_MASK; // Set FLT_UV bit
    }

    if (voltage >= FLT_OV_THRESHOLD)
    {
        status |= FLT_OV_MASK; // Set FLT_OV bit
    }

    if (voltage <= FLT_SHORT_THRESHOLD)
    {
        status |= FLT_SHORT_MASK; // Set FLT_SHORT bit
    }

    // Trips

    if (voltage <= TRIP_LLIM_THRESHOLD)
    {
        status |= TRIP_LLIM_MASK; // Set TRIP_LLIM bit
    }
    else
    {
        status &= ~TRIP_LLIM_MASK; // Clear TRIP_LLIM bit
    }

    if (voltage >= TRIP_HLIM_THRESHOLD)
    {
        status |= TRIP_HLIM_MASK; // Set TRIP_HLIM bit
    }
    else
    {
        status &= ~TRIP_HLIM_MASK; // Clear TRIP_HLIM bit
    }

    // Warns

    if (voltage >= WARN_HIGH_V_THRESHOLD)
    {
        status |= WARN_HIGH_V_MASK; // Set WARN_HIGH_V bit
    }
    else
    {
        status &= ~WARN_HIGH_V_MASK; // Clear WARN_HIGH_V bit
    }

    if (voltage <= WARN_LOW_V_THRESHOLD)
    {
        status |= WARN_LOW_V_MASK; // Set WARN_LOW_V bit
    }
    else
    {
        status &= ~WARN_LOW_V_MASK; // Clear WARN_LOW_V bit
    }

    return status;
}

// Helper function for ANA_analyzeModules()
int findModuleTempState(int status, float temp)
{
    // Some conditionals here have no else because faults don't clear

    // Faults
    if (temp >= FLT_OT_THRESHOLD)
    {
        status |= FLT_OT_MASK; // Set FLT_OT bit
    }

    if (temp >= FLT_TEMP_RANGE_HIGH_THRESHOLD || temp <= FLT_TEMP_RANGE_LOW_THRESHOLD)
    {
        status |= FLT_TEMP_RANGE_MASK; // Set FLT_TEMP_RANGE bit
    }

    // Trips

    if (temp >= TRIP_CHARGE_OT_THRESHOLD)
    {
        status |= TRIP_CHARGE_OT_MASK; // Set TRIP_CHARGE_OT bit
    }
    else
    {
        status &= ~TRIP_CHARGE_OT_MASK; // Clear TRIP_CHARGE_OT bit
    }

    // Warns

    if (temp >= WARN_HIGH_T_THRESHOLD)
    {
        status |= WARN_HIGH_T_MASK; // Set WARN_HIGH_T bit
    }
    else
    {
        status &= ~WARN_HIGH_T_MASK; // Clear WARN_HIGH_T bit
    }

    if (temp <= WARN_LOW_T_THRESHOLD)
    {
        status |= WARN_LOW_T_MASK; // Set WARN_LOW_T bit
    }
    else
    {
        status &= ~WARN_LOW_T_MASK; // Clear WARN_LOW_T bit
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
