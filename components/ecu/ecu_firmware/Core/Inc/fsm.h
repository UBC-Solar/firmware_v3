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

// State Functions:
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
void DASH_MCB_on();
void MDU_on();
void TELEM_on();
void AMB_on();
void ECU_monitor();
void fault();


/*============================================================================*/
/* FUNCTION PROTOTYPES */
void FSM_init();
void FSM_run();

#endif /* __FSM_H */
