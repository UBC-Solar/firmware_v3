/**
 *  @file fsm.h
 *  @brief Header file for the main finite state machine of the BMS
 *
 *  @date 2020/05/02
 *  @author Andrew Hanlon (a2k-hanlon)
 */

#ifndef INC_FSM_H_
#define INC_FSM_H_

#include "stm32f3xx_hal.h"
#include "ltc6811_btm.h"
#include "control.h"
#include "thermistor.h"
#include "ltc6811_btm_bal.h"
#include "Balancing_Skeleton.h"
#include "analysis.h"

/*============================================================================*/
/* ENUMERATIONS */

typedef enum
{
    FSM_RESET = 0,
    FSM_NORMAL,
    FSM_FAULT_COMM,
    FSM_FAULT_GENERAL
    // FSM_<STATE> ... Add states here
} FSM_state_t;

/*============================================================================*/
/* CONFIGURABLE PARAMETERS */

#define FSM_MIN_UPDATE_INTERVAL 500 // milliseconds - time between reads of
// voltage and temperature data, analysis of measurements, and updates to
// balancing, status, etc. Make sure this is no longer than ~2 seconds or the
// LTC6813 will be reset by its watchdog timer!

/*============================================================================*/
/* PUBLIC VARAIBLES */
unsigned int uptime; // time in seconds since FSM started

/*============================================================================*/
/* FUNCTION PROTOTYPES */

void FSM_init(void);
void FSM_run(BTM_PackData_t * pack, BTM_BAL_dch_setting_pack_t* dch_setting_pack);

#endif /* INC_FSM_H_ */
