/**
 * @file fsm.h
 * @brief Header file for the finite state machine of the ECU
 * 
 * @date 2021/01/25
 * @author Blake Shular (blake-shular)
 */

#ifndef __FSM_H
#define __FSM_H

/*============================================================================*/
/* Global Variables */
unsigned int * ADC_supp_batt_adc;
unsigned int * ADC_motor_adc;
unsigned int * ADC_array_adc;

/*============================================================================*/
/* FUNCTION PROTOTYPES */
void FSM_init();
void FSM_run();

#endif /* __FSM_H */
