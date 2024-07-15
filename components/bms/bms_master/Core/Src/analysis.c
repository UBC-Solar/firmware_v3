/**
 *  @file analysis.c
 *  @brief Analysis functions to determine charge and temperature states of battery
 *
 *  @date 2020/07/10
 *  @author Andrew Hanlon (a2k-hanlon)
 */

#include "analysis.h"
#include "bms_main.h"

/*============================================================================*/
/* PRIVATE FUNCTION IMPLEMENTATIONS */

/**
 * @brief Generates a single BMS status code as an accumulation of all modules' statuses
 *
 * @param[inout] pack Pack data structure to read module statuses from and write new pack status code to
 */
void mergeModuleStatusCodes(Pack_t *pack)
{
    Pack_BatteryStatusCode_t status_result = {.raw = 0}; // Start with a clean code

    // Just OR together status codes from all modules
    for (uint32_t module_num = 0; module_num < PACK_NUM_BATTERY_MODULES; module_num++)
    {
        status_result.raw |= pack->module[module_num].status.raw;
    }

    // Force pack-wide flags to persist (faults triggered at the module level persist at that level
    // an will be captured by the combining of codes above)
    status_result.raw |= pack->status.raw & PACK_TOP_LEVEL_STATUS_BITS_MASK;

    pack->status.raw = status_result.raw;
}

/**
 * @brief Writes the WARN_REGEN_OFF status bit to the pack status
 *
 * @param[inout] pack Pack data structure in which to update pack status code
 */
void writePackRegenStatus(Pack_t *pack)
{
    Pack_BatteryStatusCode_t *status = &(pack->status);

    if ( // Any faults, CHARGE_OT or HLIM
        PACK_ANY_FAULTS_SET(*status) ||
        status->bits.charge_over_temperature_limit ||
        status->bits.hlim
    )
    {
        status->bits.warning_regen_off = true;
    }
    else
    {
        status->bits.warning_regen_off = false;
    }
}

void findModuleTempState(Pack_BatteryStatusCode_t *status, float temperature)
{
    // Some conditionals here have no else because faults don't clear

    // Faults
    if (temperature >= FLT_OT_DCH_THRESHOLD)
    {
        status->bits.fault_over_temperature = true;
    }

    if (temperature >= FLT_OT_CH_THRESHOLD && ecu_data.is_charging == true){
        status->bits.fault_over_temperature = true;
    }

    if (temperature >= FLT_TEMP_RANGE_HIGH_THRESHOLD || temperature <= FLT_TEMP_RANGE_LOW_THRESHOLD)
    {
        status->bits.fault_temperature_expected_range = true;
    }

    // Trips

    if (temperature >= TRIP_CHARGE_OT_THRESHOLD)
    {
        status->bits.charge_over_temperature_limit = true;
    }
    else if (temperature < (TRIP_CHARGE_OT_THRESHOLD - TEMPERATURE_THRESHOLD_HYSTERESIS))
    {
        status->bits.charge_over_temperature_limit = false;
    }

    // Warns

    if (temperature >= WARN_HIGH_T_THRESHOLD)
    {
        status->bits.warning_high_temperature = true;
    }
    else if (temperature < (WARN_HIGH_T_THRESHOLD - TEMPERATURE_THRESHOLD_HYSTERESIS))
    {
        status->bits.warning_high_temperature = false;
    }

    if (temperature <= WARN_LOW_T_THRESHOLD)
    {
        status->bits.warning_low_temperature = true;
    }
    else if (temperature > (WARN_LOW_T_THRESHOLD + TEMPERATURE_THRESHOLD_HYSTERESIS))
    {
        status->bits.warning_low_temperature = false;
    }
}

void findModuleVoltState(Pack_BatteryStatusCode_t *status, uint16_t voltage)
{
    // Some conditionals here have no else because faults don't clear

    // Faults

    if (voltage <= FLT_UV_THRESHOLD)
    {
        status->bits.fault_under_voltage = true;
    }

    if (voltage >= FLT_OV_THRESHOLD)
    {
        status->bits.fault_over_voltage = true;
    }

    if (voltage <= FLT_SHORT_THRESHOLD)
    {
        status->bits.fault_short = true;
    }

    // Trips

    if (voltage <= TRIP_LLIM_THRESHOLD)
    {
        status->bits.llim = true;
    }
    else if (voltage > (TRIP_LLIM_THRESHOLD + VOLTAGE_THRESHOLD_HYSTERESIS))
    {
        status->bits.llim = false;
    }

    if (voltage >= TRIP_HLIM_THRESHOLD)
    {
        status->bits.hlim = true;
    }
    else if (voltage < (TRIP_HLIM_THRESHOLD - VOLTAGE_THRESHOLD_HYSTERESIS))
    {
        status->bits.hlim = false;
    }

    // Warnings

    if (voltage >= WARN_HIGH_V_THRESHOLD)
    {
        status->bits.warning_high_voltage = true;
    }
    else if (voltage < (WARN_HIGH_V_THRESHOLD - VOLTAGE_THRESHOLD_HYSTERESIS))
    {
        status->bits.warning_high_voltage = false;
    }

    if (voltage <= WARN_LOW_V_THRESHOLD)
    {
        status->bits.warning_low_voltage = true;
    }
    else if (voltage > (WARN_LOW_V_THRESHOLD + VOLTAGE_THRESHOLD_HYSTERESIS))
    {
        status->bits.warning_low_voltage = false;
    }
}

/**
 * @brief Determines voltage- and temperature-based state of each module in the pack
 *
 * @param[inout] pack Pack data structure to update module statuses in
 */
void analyzeModules(Pack_t *pack)
{
    Pack_Module_t *module; // pointer to module being analyzed

    for (uint32_t module_num = 0; module_num < PACK_NUM_BATTERY_MODULES; module_num++)
    {
        module = &(pack->module[module_num]);
        // Update module status
        findModuleVoltState(&(module->status), module->voltage);
        findModuleTempState(&(module->status), module->temperature);
    }
}

/*============================================================================*/
/* PUBLIC FUNCTION IMPLEMENTATIONS */

/**
 * @brief Performs analysis of all modules, then writes the pack status
 *
 * @note This function is not responsible for triggering the following bits:
 *  FLT_COMM
 *  FLT_TEST
 *  FLT_ISOL
 *  FLT_DOC_COC
 *  TRIP_BAL
 *
 * @note This function must be called after capturing battery voltage and temperature measurements,
 * and updating balancing states of the modules
 *
 * @param[inout] pack Pack data structure in which to update module and pack status codes
 */
void ANA_analyzePack(Pack_t *pack)
{
    analyzeModules(pack);
    mergeModuleStatusCodes(pack);
    writePackRegenStatus(pack);
}

/**
 * @brief Updates the balancing status bit in the status code for a battery module
 *
 * @param[out] module Pack module to update
 * @param[in] discharge_active New discharge/balancing state
*/
void ANA_writeBalStatus(Pack_Module_t *module, bool discharge_active)
{
    module->status.bits.balancing_active = discharge_active;
}

/**
 * @brief Finds the value of the highest module temperature in the battery pack
 *
 * @param pack Data pack to search for highest module temperature
 * @return Floating point value in degrees C of the hottest module's temperature
 */
float ANA_findHighestModuleTemp(Pack_t *pack)
{
    float temperature = 0.0;
    float max_temperature = 0.0;

    for (uint32_t module_num = 0; module_num < PACK_NUM_BATTERY_MODULES; module_num++)
    {
            temperature = pack->module[module_num].temperature;
            if (temperature > max_temperature)
            {
                max_temperature = temperature;
            }
    }

    return max_temperature;
}
