/**
 * @file ltc6811_btm_bal.c
 * @brief Functions for controlling LTC6811s' balancing operations
 *
 * @date 2020/05/09
 * @author
 */

#include "ltc6811_btm_bal.h"

/**
 * @brief Copy all discharge settings from one BTM_BAL_dch_setting_pack_t instance to another
 *
 * @param[in] dch_pack_source discharge setting pack to copy from
 * @param[out] dch_pack_target discharge setting pack to write
 */
void BTM_BAL_copyDchPack(
    BTM_BAL_dch_setting_pack_t* dch_pack_source,
    BTM_BAL_dch_setting_pack_t* dch_pack_target)
{
    for(int stack_num = 0; stack_num < BTM_NUM_DEVICES; stack_num++)
    {
        for(int module_num = 0; module_num < BTM_NUM_MODULES; module_num++)
        {
            dch_pack_target->stack[stack_num].module_dch[module_num] =
                dch_pack_source->stack[stack_num].module_dch[module_num];
        }
    }
    return;
}

/**
 * @brief Writes discharging settings to LTC6811s to trigger module discharging
 *
 * @attention dch_setting_pack must be complete. Every single module's discharge
 *  circuit will be turned on or off according to this structure.
 *
 * @param[out] pack The main pack data structure of the program to write balancing status flags to
 * @param[in] dch_setting_pack The discharge settings for the entire pack.
 */
void BTM_BAL_setDischarge(
    BTM_PackData_t* pack,
    BTM_BAL_dch_setting_pack_t* dch_setting_pack)
{
    uint8_t cfgr_to_write[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE] = {0};
    BTM_module_bal_status_t module_bal_status = DISCHARGE_OFF;

    for(int ic_num = 0; ic_num < BTM_NUM_DEVICES; ic_num++)
    {
        // Iterate through the bytes of the CFGR register value for each LTC6811

        // Don't change the first 4 bytes of the CFGR register group,
        // so copy the existing configuration
        for(int byte_num = 0; byte_num < (BTM_REG_GROUP_SIZE - 2); byte_num++)
        {
            cfgr_to_write[ic_num][byte_num] =
                pack->stack[ic_num].cfgr[byte_num];
        }

        // Do change the last 2 bytes of CFGR, the balancing settings
        // 5th byte - DCH setting for first 8 modules
        for(int i = 0; i < 8; i++)
        {
            module_bal_status =
                dch_setting_pack->stack[ic_num].module_dch[i];
            // Set DCH bit for this module (cell)
            cfgr_to_write[ic_num][BTM_REG_GROUP_SIZE - 2] |=
                module_bal_status << i;
            // Update balancing status in pack data structure
            pack->stack[ic_num].module[i].bal_status = module_bal_status;
        }

        // 6th byte - DCH etting for last 4 modules
        for(int j = 0; j < 4; j++)
        {
            module_bal_status =
                dch_setting_pack->stack[ic_num].module_dch[j + 8];
            // Set DCH bit for this module (cell)
            cfgr_to_write[ic_num][BTM_REG_GROUP_SIZE - 1] |=
                module_bal_status << j;
            // Update balancing status in pack data structure
            pack->stack[ic_num].module[j + 8].bal_status = module_bal_status;
        }
    }

    BTM_writeRegisterGroup(CMD_WRCFGA, cfgr_to_write);
    return;
}
