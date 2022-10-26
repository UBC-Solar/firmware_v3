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

typedef enum
{
    DISCHARGE_OFF = 0,
    DISCHARGE_ON = 1
} BTM_module_bal_status_t;

/*============================================================================*/
/* STRUCTURES */

/*============================================================================*/
/* FUNCTION PROTOTYPES */

void BTM_BAL_setDischarge(BTM_PackData_t *pack);

#endif /* INC_LTC6813_BTM_BAL_H_ */
