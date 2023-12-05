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
    PC_DCDC,
    DCDC_PLUS,
    DISABLE_MDU_DCH,
    CHECK_LLIM,
    WAIT_FOR_PC,
    LLIM_CLOSED,
    CHECK_HLIM,
    TELEM_ON,
    DASH_ON,
    MCB_ON,
    MDU_ON,
    SPAR1_ON,
    AMB_ON,
    MONITORING,
    FAULT
} FSM_state_t;

/*============================================================================*/
/* GLOBAL VARIABLES */

static unsigned int last_tick;
static FSM_state_t FSM_state;
static bool LVS_ALREADY_ON = false;
static bool last_HLIM_status;
static bool last_LLIM_status;

/*============================================================================*/
/* DEFINED CONSTANTS */

// Time intervals are in milliseconds
#define BMS_STARTUP_INTERVAL 5000
#define MESSAGE_INTERVAL_0X3F4 1000
#define MESSAGE_INTERVAL_0X450 200
#define MDU_DCH_INTERVAL 500
#define SHORT_INTERVAL 300
#define MDU_PC_INTERVAL 705 
#define FLT_BLINK_INTERVAL 200
#define LVS_INTERVAL 200

#define SUPP_LIMIT 10500 //mV
#define COC_THRESHOLD 22000 //mA
#define DOC_THRESHOLD -65000 //mA
#define LOW false
#define HIGH true

#define CONTACTOR_CLOSED HIGH
#define CONTACTOR_OPEN LOW

#define REQ_CONTACTOR_CLOSE LOW
#define REQ_CONTACTOR_OPEN HIGH

// Helper Functions:
bool timer_check(unsigned int millis);

/*============================================================================*/
/* STATE MACHINE FUNCTIONS */

void FSM_reset();
void BMS_powerup();
void BMS_ready();
void DCDC_minus();
void DCDC_plus();
void disable_MDU_DCH();
void check_LLIM();
void PC_wait();
void LLIM_closed();
void check_HLIM();
void TELEM_on();
void DASH_on();
void MCB_on();
void MDU_on();
void SPAR1_on();
void AMB_on();
void ECU_monitor();
void fault();

/*============================================================================*/
/* STATE TABLE */

static void (*FSM_state_table[])(void) = {
    FSM_reset,
    BMS_powerup,
    BMS_ready,
    DCDC_minus,
    DCDC_plus,
    disable_MDU_DCH,
    check_LLIM,
    PC_wait,
    LLIM_closed,
    check_HLIM,
    TELEM_on,
    DASH_on,
    MCB_on,
    MDU_on,
    SPAR1_on,
    AMB_on,
    ECU_monitor,
    fault};

/*============================================================================*/
/* FUNCTION PROTOTYPES */
void FSM_Init();
void FSM_run();

#endif /* __FSM_H */
