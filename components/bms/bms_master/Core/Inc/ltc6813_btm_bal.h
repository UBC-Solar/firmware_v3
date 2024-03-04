/**
 * @file ltc6813_btm_bal.h
 * @brief Functions for control of LTC6813s' passive balancing discharge operations
 *
 * DCH is short for discharge, and this refers to discharging for balancing
 * rather than discharging in general.
 *
 * @date 2020/08/18
 * @author Andrew Hanlon (a2k-hanlon)
 */

#ifndef INC_LTC6813_BTM_BAL_H_
#define INC_LTC6813_BTM_BAL_H_

#include <stdbool.h>
#include "pack.h"

/*============================================================================*/
/* FUNCTION PROTOTYPES */

void BTM_BAL_setDischarge(Pack_t *pack, const bool discharge_setting[PACK_NUM_BATTERY_MODULES]);

#endif /* INC_LTC6813_BTM_BAL_H_ */
