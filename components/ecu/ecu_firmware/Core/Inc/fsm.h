/**
 * @file fsm.h
 * @brief Header file for the finite state machine of the ECU
 *
 * @date 2021/01/25
 * @author Blake Shular (blake-shular)
 */

#ifndef __FSM_H
#define __FSM_H

#include "main.h"
#include <stdbool.h>
#include "adc.h"
#include "can.h"
#include "stm32f1xx_hal.h"

/*============================================================================*/
/* PRIVATE FUNCTION PROTOTYPES */
/*============================================================================*/
/* STATE MACHINE STATES */

typedef enum
{
    FSM_RESET = 0,
    WAIT_FOR_BMS_POWERUP,
    WAIT_FOR_BMS_READY,
    HV_CONNECT,
    SWAP_DCDC,
    DISABLE_MDU_DCH,
    CHECK_LLIM,
    WAIT_FOR_PC,
    LLIM_CLOSED,
    CHECK_HLIM,
    TELEM_ON,
    MEM_ON,
    DASH_ON,
    MCB_ON,
    MDU_ON,
    AMB_ON,
    MONITORING,
    FAULT
} FSM_state_t;

/*============================================================================*/
/* DEFINED CONSTANTS */

// Time intervals are in milliseconds
#define BMS_STARTUP_INTERVAL 5000
#define MESSAGE_INTERVAL_0X450 200
#define MDU_DCH_INTERVAL 500
#define SHORT_INTERVAL 200
#define MDU_PC_INTERVAL 1500
#define FLT_BLINK_INTERVAL 200
#define LVS_INTERVAL 100

#define SUPP_LIMIT 10500 // mV
#define DOC_COC_FAULT 0
#define ESTOP_ACTIVE_FAULT 0
#define LOW false
#define HIGH true

#define CONTACTOR_CLOSED HIGH
#define CONTACTOR_OPEN LOW

#define REQ_CONTACTOR_CLOSE LOW
#define REQ_CONTACTOR_OPEN HIGH

/*============================================================================*/
//  Threshold Values

#define DOC_WARNING_THRESHOLD 55000 // mA
#define COC_WARNING_THRESHOLD -18000 // mA

/*============================================================================*/
/* STATE MACHINE FUNCTIONS */

void FSM_reset();
void BMS_powerup();
void BMS_ready();
void HV_Connect();
void swap_DCDC();
void disable_MDU_DCH();
void check_LLIM();
void PC_wait();
void LLIM_closed();
void check_HLIM();
void TELEM_on();
void MEM_on();
void DASH_on();
void MCB_on();
void MDU_on();
void AMB_on();
void ECU_monitor();
void fault();

/*============================================================================*/
/* STATE TABLE */

static void (*FSM_state_table[])(void) = {
    FSM_reset,
    BMS_powerup,
    BMS_ready,
    HV_Connect,
    swap_DCDC,
    disable_MDU_DCH,
    check_LLIM,
    PC_wait,
    LLIM_closed,
    check_HLIM,
    TELEM_on,
    MEM_on,
    DASH_on,
    MCB_on,
    MDU_on,
    AMB_on,
    ECU_monitor,
    fault};

/*============================================================================*/
/* FUNCTION PROTOTYPES */

// FSM
void FSM_Init();
void FSM_run();

// ADC
void FSM_ADC_LevelOutOfWindowCallback();
void FSM_ESTOPActivedCallback();

#endif /* __FSM_H */
