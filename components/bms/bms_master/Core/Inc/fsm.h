/**
 *  @file fsm.h
 *  @brief Header file for the main finite state machine of the BMS
 *
 *  @date 2020/05/02
 *  @author Andrew Hanlon (a2k-hanlon)
 */

#ifndef __FSM_H
#define __FSM_H

#include "stm32f1xx_hal.h"
#include "ltc6813_btm.h"
#include "control.h"
#include "ltc6813_btm_temp.h"
#include "ltc6813_btm_bal.h"
#include "btm_bal_settings.h"
#include "analysis.h"
#include "selftest.h"
#include "soc.h"

/*============================================================================*/
/* ENUMERATIONS */

// State machine states
typedef enum
{
    FSM_RESET = 0,
    FSM_MEASURE,
    FSM_CTRL,
    FSM_CAN,
    FSM_FAULT
} FSM_state_t;

/*============================================================================*/
/* GLOBAL VARAIBLES */
unsigned int uptime; // time in seconds since FSM started

/*============================================================================*/
/* DEFINED CONSTANTS */

#define FSM_MIN_UPDATE_INTERVAL 500 // milliseconds - time between reads of
// voltage and temperature data, analysis of measurements, and updates to
// balancing, status, etc. Make sure this is no longer than ~2 seconds or the
// LTC6813 will be reset by its watchdog timer!

/*============================================================================*/
/* STATE MACHINE FUNCTIONS */
void FSM_reset(BTM_PackData_t *pack);
void FSM_measure(BTM_PackData_t *pack);
void FSM_ctrl(BTM_PackData_t *pack);
void FSM_can(BTM_PackData_t *pack);
void FSM_fault(BTM_PackData_t *pack);

/*============================================================================*/
/* STATE TABLE */

void (*FSM_state_table[])(BTM_PackData_t *pack) = {
    FSM_reset,
    FSM_measure,
    FSM_ctrl,
    FSM_can,
    FSM_fault};

/*============================================================================*/
/* FUNCTION PROTOTYPES */

void FSM_init(BTM_PackData_t *pack);
void FSM_run(BTM_PackData_t *pack);

#endif /* __FSM_H */
