/**
 * @file fsm.h
 * @brief Header file for the finite state machine of the ECU
 * 
 * @date 2021/01/25
 * @author Blake Shular (blake-shular)
 */

#ifndef __FSM_H
#define __FSM_H

typedef enum {
    RESET,
    WAIT_FOR_BMS_POWERUP,
    WAIT_FOR_BMS_READY,
    PC_DCDC,
    DCDC_PLUS,
    DISABLE_MDU_DCH,
    CLOSE_NEG,
    WAIT_FOR_PC,
    LLIM_CLOSED,
    CHECK_HLIM,
    LVS_ON,
    MONITORING,
    FAULT
} FSM_state;

/*============================================================================*/
/* FUNCTION PROTOTYPES */
void FSM_init();
void FSM_run();

#endif /* __FSM_H */
