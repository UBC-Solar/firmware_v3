/**
 * @file ltc6813_btm_bal.c
 * @brief Functions for controlling LTC6813s' passive balancing discharge operations
 *
 * @date 2020/08/18
 * @author Andrew Hanlon (a2k-hanlon)
 */

#include "ltc6813_btm_bal.h"
#include "ltc6813_btm.h"
#include "analysis.h"

/*============================================================================*/
/* DEFINITIONS */

#define CFGAR4_DCC_BITS_BITMASK 0xFFU // All bits of 5th byte
#define CFGAR5_DCC_BITS_BITMASK 0x0FU // Lower 4 bits of 6th byte
#define CFGBR0_DCC_BITS_BITMASK 0xF0U // Upper 4 bits of 1st byte
#define CFGBR1_DCC_BITS_BITMASK 0x03U // Lowest 2 bits of 2nd byte

/*============================================================================*/
/* PUBLIC FUNCTION DEFINITIONS */

/**
 * @brief Writes discharging settings to LTC6813s to perform passive balancing of modules
 *
 * Unused cell inputs of the LTC6813s will always have their discharge settings turned off.
 * Also note that the DCTO (discharge timeout) value in Configuration Register Group A will always be set to 0 (disabled)
 *
 * @attention BTM_Init() should be called before calling this function so that other config. register fields are set properly
 *
 * @param[out] pack Pack data structure in which to update balancing status flags
 * @param[in] discharge_setting Array of toggles to enable (true) or disable (false) balancing discharge of each module
 */
void BTM_BAL_setDischarge(Pack_t *pack, bool discharge_setting[PACK_NUM_BATTERY_MODULES])
{
    uint32_t discharge_bitmask[BTM_NUM_DEVICES] = {0};
    uint32_t device_num;
    uint32_t cell_num;

    for (uint32_t module_num = 0; module_num < PACK_NUM_BATTERY_MODULES; module_num++)
    {
        // Construct discharge bitmask using pack module mapping and given discharge settings
        device_num = Pack_module_mapping[module_num].device_num;
        cell_num = Pack_module_mapping[module_num].cell_num;
        discharge_bitmask[device_num] |= ((uint32_t) discharge_setting[module_num]) << cell_num;
    }

    // Update balancing status in pack data
    ANA_writeBalStatus(pack, discharge_setting);

    for (device_num = 0; device_num < BTM_NUM_DEVICES; device_num++)
    {
        // Clear existing DCC (discharge control) bits of the configuration registers
        // and rewrite them with the new discharge settings

        // 5th byte of CFGRA - DCH setting for cell inputs 1-8 (first 8 modules)
        BTM_data.cfgra[device_num][4] &= ~CFGAR4_DCC_BITS_BITMASK;
        BTM_data.cfgra[device_num][4] |= (uint8_t) (discharge_bitmask[device_num] & CFGAR4_DCC_BITS_BITMASK);

        // 6th byte of CFGRA - DCH setting for cell inputs 9-12
        BTM_data.cfgra[device_num][5] &= ~CFGAR5_DCC_BITS_BITMASK;
        BTM_data.cfgra[device_num][5] |= (uint8_t) ((discharge_bitmask[device_num] >> 8) & CFGAR5_DCC_BITS_BITMASK);

        // 1st byte of CFGRB - DCH setting for cell inputs 13-16
        BTM_data.cfgrb[device_num][0] &= ~CFGBR0_DCC_BITS_BITMASK;
        BTM_data.cfgrb[device_num][0] |= (uint8_t) ((discharge_bitmask[device_num] >> 12) & CFGBR0_DCC_BITS_BITMASK);

        // 2nd byte of CFGRB - DCH setting for cell inputs 17 and 18
        BTM_data.cfgrb[device_num][1] &= ~CFGBR1_DCC_BITS_BITMASK;
        BTM_data.cfgrb[device_num][1] |= (uint8_t) ((discharge_bitmask[device_num] >> 16) & CFGBR1_DCC_BITS_BITMASK);
    }

    BTM_writeRegisterGroup(CMD_WRCFGA, BTM_data.cfgra);
    BTM_writeRegisterGroup(CMD_WRCFGB, BTM_data.cfgrb);
}
