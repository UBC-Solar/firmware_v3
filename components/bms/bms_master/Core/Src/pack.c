/**
 *  @file pack.c
 *  @brief Common battery pack data structures and module mapping to hardware
 *
 *  @date 2023/03/18
 *  @author Tigran Hakobyan (Tik-Hakobyan)
 */

#include "pack.h"
#include <float.h>

const Pack_ModuleIndex_t Pack_module_mapping[PACK_NUM_BATTERY_MODULES] = {
    [ 0] = {.device_num = 0, .cell_num =  0},
    [ 1] = {.device_num = 0, .cell_num =  1},
    [ 2] = {.device_num = 0, .cell_num =  2},
    [ 3] = {.device_num = 0, .cell_num =  3},
    [ 4] = {.device_num = 0, .cell_num =  4},
    [ 5] = {.device_num = 0, .cell_num =  5},
    [ 6] = {.device_num = 0, .cell_num =  6},
    [ 7] = {.device_num = 0, .cell_num =  7},
    [ 8] = {.device_num = 0, .cell_num =  8},
    [ 9] = {.device_num = 0, .cell_num =  9},
    [10] = {.device_num = 0, .cell_num = 10},
    [11] = {.device_num = 0, .cell_num = 12}, // Cell index 11 (C12) unused
    [12] = {.device_num = 0, .cell_num = 13},
    [13] = {.device_num = 0, .cell_num = 14},
    [14] = {.device_num = 0, .cell_num = 15},
    [15] = {.device_num = 0, .cell_num = 16}, // Cell index 17 (C18) unused

    [16] = {.device_num = 1, .cell_num =  0},
    [17] = {.device_num = 1, .cell_num =  1},
    [18] = {.device_num = 1, .cell_num =  2},
    [19] = {.device_num = 1, .cell_num =  3},
    [20] = {.device_num = 1, .cell_num =  4},
    [21] = {.device_num = 1, .cell_num =  5},
    [22] = {.device_num = 1, .cell_num =  6},
    [23] = {.device_num = 1, .cell_num =  7},
    [24] = {.device_num = 1, .cell_num =  8},
    [25] = {.device_num = 1, .cell_num =  9},
    [26] = {.device_num = 1, .cell_num = 10},
    [27] = {.device_num = 1, .cell_num = 12}, // Cell index 11 (C12) unused
    [28] = {.device_num = 1, .cell_num = 13},
    [29] = {.device_num = 1, .cell_num = 14},
    [30] = {.device_num = 1, .cell_num = 15},
    [31] = {.device_num = 1, .cell_num = 16}, // Cell index 17 (C18) unused
};

/**
 * @brief Returns the total battery pack voltage
 *
 * @param[in] pack Battery pack data structure to compute pack voltage from
 * @returns Battery pack voltage at the same scale factor as the pack module voltage field
 */
uint32_t Pack_GetPackVoltage(Pack_t *pack)
{
    uint32_t pack_voltage = 0;
    for(int module_num = 0; module_num < PACK_NUM_BATTERY_MODULES; module_num++)
    {
        pack_voltage += (uint32_t) pack->module[module_num].voltage;
    }
    return pack_voltage;
}

/**
 * @brief Returns the battery pack state of charge (SOC) estimate
 *
 * @param[in] pack Battery pack data structure to compute pack SOC from
 * @returns Battery pack SOC as a percentage (0.0 - 100.0)
 */
float Pack_GetPackStateOfCharge(Pack_t *pack)
{
    float pack_soc = FLT_MAX;
    // The SOC of the pack is the SOC of the least charged module
    for(int module_num = 0; module_num < PACK_NUM_BATTERY_MODULES; module_num++)
    {
        if (pack->module[module_num].state_of_charge < pack_soc)
        {
            pack_soc = pack->module[module_num].state_of_charge;
        }
    }
    return pack_soc;
}
