/**
 * @file ltc6813_btm_bal.h
 * @brief Header file for control of LTC6813s' balancing operations
 *
 * DCH is short for discharge, and this refers to discharging for balancing
 * rather than discharging in general.
 *
 *  This file was originally modified from ltc6811_btm_bal.h
 *
 * @date 2020/08/18
 * @author Andrew Hanlon (a2k-hanlon)
 */

#ifndef INC_LTC6813_BTM_BAL_H_
#define INC_LTC6813_BTM_BAL_H_

#include "ltc6813_btm.h"

/*============================================================================*/
/* ENUMERATIONS */

typedef enum {
    DISCHARGE_OFF = 0,
    DISCHARGE_ON  = 1
} BTM_module_bal_status_t;

/*============================================================================*/
/* STRUCTURES */

struct BTM_BAL_dch_setting_stack {
    BTM_module_bal_status_t module_dch[BTM_NUM_MODULES];
};

typedef struct {
    struct BTM_BAL_dch_setting_stack stack[BTM_NUM_DEVICES];
} BTM_BAL_dch_setting_pack_t;

/*============================================================================*/
/* FUNCTION PROTOTYPES */

void BTM_BAL_initDchPack(BTM_BAL_dch_setting_pack_t* dch_pack);
void BTM_BAL_copyDchPack(
    BTM_BAL_dch_setting_pack_t* dch_pack_source,
    BTM_BAL_dch_setting_pack_t* dch_pack_target
);
void BTM_BAL_setDischarge(
    BTM_PackData_t* pack,
    BTM_BAL_dch_setting_pack_t* pack_dch_setting
);

#endif /* INC_LTC6813_BTM_BAL_H_ */
