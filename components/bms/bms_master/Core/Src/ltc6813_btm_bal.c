/**
 * @file ltc6813_btm_bal.c
 * @brief Functions for controlling LTC6813s' balancing operations
 *
 *  This file was originally modified from ltc6811_btm_bal.c
 *
 * @date 2020/08/18
 * @author Andrew Hanlon (a2k-hanlon)
 */

#include "ltc6813_btm_bal.h"
#include "analysis.h"

// Private function prototype
void writeBalStatusBit(struct BTM_module * module, BTM_module_bal_status_t bal_status);


/**
 * @brief Initialize all discharge settings of a setting pack to DISCHARGE_OFF
 *
 * @param[out] dch_pack discharge setting pack to write
 */
void BTM_BAL_initDchPack(BTM_BAL_dch_setting_pack_t* dch_pack)
{
    for(int stack_num = 0; stack_num < BTM_NUM_DEVICES; stack_num++)
    {
        for(int module_num = 0; module_num < BTM_NUM_MODULES; module_num++)
        {
            dch_pack->stack[stack_num].module_dch[module_num] = DISCHARGE_OFF;
        }
    }
    return;
}

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
 * @brief Writes discharging settings to LTC6813s to trigger module discharging
 *
 * @attention dch_setting_pack must be complete. Every single module's discharge
 *  circuit will be turned on or off according to this structure.
 *
 * Disabled modules will always have their discharge settings turned off. Also
 * note that the DCTO value in Config. Reg. Group. A will always be set to 0
 *
 * @param[in/out] pack The main pack data structure of the program to write
 *  balancing status flags to and read module enable flags from
 * @param[in] dch_setting_pack The discharge settings for the entire pack.
 */
void BTM_BAL_setDischarge(
    BTM_PackData_t* pack,
    BTM_BAL_dch_setting_pack_t* dch_setting_pack)
{
    uint8_t cfgra_to_write[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE] = {0};
    uint8_t cfgrb_to_write[BTM_NUM_DEVICES][BTM_REG_GROUP_SIZE] = {0};
    BTM_module_bal_status_t module_bal_status = DISCHARGE_OFF;
    BTM_module_enable_t module_enable = MODULE_DISABLED;
    int module_i = 0; // index counter

    for(int ic_num = 0; ic_num < BTM_NUM_DEVICES; ic_num++)
    {
        // Iterate through the bytes of both CFGR registers for each LTC6813

        // Don't change all the bytes of the CFGRA/B register groups,
        // so copy most of the existing configuration
        for(int byte_num = 0; byte_num < BTM_REG_GROUP_SIZE; byte_num++)
        {
            if (byte_num < 4) { // First 4 bytes
                cfgra_to_write[ic_num][byte_num] =
                    pack->stack[ic_num].cfgra[byte_num];
            }
            cfgrb_to_write[ic_num][byte_num] =
                pack->stack[ic_num].cfgrb[byte_num];
        }
        // Clear DCC bits in CFGRB
        cfgrb_to_write[ic_num][0] &= 0x0F; // Clear upper 4 bits of 1st byte
        cfgrb_to_write[ic_num][1] &= 0xFC; // Clear lowest 2 bits of 2nd byte

        // Now change the DCC bits, the balancing (discharge) settings

        // 5th byte of CFGRA - DCH setting for modules 1-8 (first 8 modules)
        for(int i = 0; i < 8; i++)
        {
            module_bal_status =
                dch_setting_pack->stack[ic_num].module_dch[i];
            module_enable = pack->stack[ic_num].module[i].enable;
            // Set DCH bit for this module (cell)
            // but only if module is enabled
            cfgra_to_write[ic_num][4] |=
                (module_bal_status & module_enable) << i;
            // Update balancing status in pack data structure
            writeBalStatusBit(&(pack->stack[ic_num].module[module_i]), module_bal_status);
        }

        // 6th byte of CFGRA - DCH setting for modules 9-12
        for(int j = 0; j < 4; j++)
        {
            module_i = j + 8;
            module_bal_status =
                dch_setting_pack->stack[ic_num].module_dch[module_i];
            module_enable = pack->stack[ic_num].module[module_i].enable;
            // Set DCH bit for this module (cell)
            // but only if module's enable flag is MODULE_ENABLED
            cfgra_to_write[ic_num][5] |=
                (module_bal_status & module_enable) << j;
            // Update balancing status in pack data structure
            writeBalStatusBit(&(pack->stack[ic_num].module[module_i]), module_bal_status);
        }

        // 1st byte of CFGRB - DCH setting for modules 13-16
        for(int k = 0; k < 4; k++)
        {
            module_i = k + 12;
            module_bal_status =
                dch_setting_pack->stack[ic_num].module_dch[module_i];
            module_enable = pack->stack[ic_num].module[module_i].enable;
            // Set DCH bit for this module (cell)
            // but only if module's enable flag is MODULE_ENABLED
            cfgrb_to_write[ic_num][0] |=
                (module_bal_status & module_enable) << (k + 4);
            // Update balancing status in pack data structure
            writeBalStatusBit(&(pack->stack[ic_num].module[module_i]), module_bal_status);
        }

        // 2nd byte of CFGRB - DCH setting for modules 17 and 18
        for(int l = 0; l < 2; l++)
        {
            module_i = l + 16;
            module_bal_status =
                dch_setting_pack->stack[ic_num].module_dch[module_i];
            module_enable = pack->stack[ic_num].module[module_i].enable;
            // Set DCH bit for this module (cell)
            // but only if module's enable flag is MODULE_ENABLED
            cfgrb_to_write[ic_num][1] |=
                (module_bal_status & module_enable) << l;
            // Update balancing status in pack data structure
            writeBalStatusBit(&(pack->stack[ic_num].module[module_i]), module_bal_status);
        }
    }

    BTM_writeRegisterGroup(CMD_WRCFGA, cfgra_to_write);
    BTM_writeRegisterGroup(CMD_WRCFGA, cfgrb_to_write);

    return;
}


void writeBalStatusBit(struct BTM_module * module, BTM_module_bal_status_t bal_status)
{
    if (bal_status == DISCHARGE_ON)
        module->status |= BMS_TRIP_BAL; // Set TRIP_BAL bit
    else
        module->status &= ~BMS_TRIP_BAL; // Clear TRIP_BAL bit
    return;
}
