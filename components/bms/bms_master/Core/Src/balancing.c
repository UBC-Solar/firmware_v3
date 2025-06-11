/**
 *  @file balancing.c
 *  @brief Module passive balancing algorithm
 *
 *  @date 2020/08/18
 *  @author abooodeee
 */

#include "balancing.h"
#include "ltc6813_btm_bal.h"
#include <stdbool.h>

/*============================================================================*/
/* PRIVATE FUNCTION DEFINITIONS */

/**
 * @brief Obtains the minimum module voltage in a pack and its module index
 *
 * @param[out] min_voltage Minimum module voltage found in the packs
 * @param[out] module_index Index of the module with the minimum voltage
 * @param[in] pack Pack data structure to search
*/
void findMinModuleVoltage(uint16_t *min_voltage, uint32_t *module_index, Pack_t *pack)
{
    uint16_t temp_val = pack->module[0].voltage; // the first module's voltage is the first point of comparison and will change if another voltage is smaller
    uint32_t temp_loc = 0;

    for (uint32_t module_num = 1; module_num < PACK_NUM_BATTERY_MODULES; module_num++)
    {
        if (pack->module[module_num].voltage < temp_val)
        {
            temp_val = pack->module[module_num].voltage;
            temp_loc = module_num;
        }
    }

    *min_voltage = temp_val;
    *module_index = temp_loc;
}

/*============================================================================*/
/* PUBLIC FUNCTION DEFINITIONS */

/**
 * @brief Determines which battery modules should be discharged for balancing and
 * puts this balancing configuration into effect
 *
 * @param[inout] pack Pack data structure from which to read module voltages for
 * determining balancing settings and to which the balancing status is written
*/
void BAL_updateBalancing(Pack_t *pack)
{
    uint16_t min_voltage; // Minimum voltage in the pack
    uint32_t min_voltage_module; // Location of the minimum voltage in the array
    bool discharge_setting[PACK_NUM_BATTERY_MODULES] = {false};

    findMinModuleVoltage(&min_voltage, &min_voltage_module, pack);

    // Discharge all modules that are above the threshold voltage if any cell is below the threshold
    if (min_voltage < BAL_MIN_VOLTAGE_FOR_BALANCING)
    {
        for (uint32_t module_num = 0; module_num < PACK_NUM_BATTERY_MODULES; module_num++)
        {
            if (pack->module[module_num].voltage > BAL_MIN_VOLTAGE_FOR_BALANCING)
            {
                discharge_setting[module_num] = true;
            }
            else
            {
                discharge_setting[module_num] = false;
            }
        }
    }
    else // Otherwise, discharge modules as necessary down to the lowest module voltage
    {
        for (uint32_t module_num = 0; module_num < PACK_NUM_BATTERY_MODULES; module_num++)
        {
            if(pack->module[module_num].voltage > (min_voltage + BAL_VOLTAGE_DIFFERENCE_TOLERANCE))
            {
                // turn on the corresponding s
                discharge_setting[module_num] = true;
            }
            else
            {
                discharge_setting[module_num] = false;
            }
        }
    }
    
    // TODO: REMOVE LATER. TEMPORARY PATCH UNTIL MODULE 7's VOLTAGE IS FIXED
    for (int module_num = 0; module_num < PACK_NUM_BATTERY_MODULES; ++module_num)
    {
        discharge_setting[module_num] = false;
    }
    // TODO: REMOVE LATER. TEMPORARY PATCH UNTIL MODULE 7's VOLTAGE IS FIXED

    // Call BTM_BAL_setDischarge to put the balancing settings into effect
    BTM_BAL_setDischarge(pack, discharge_setting);
}
