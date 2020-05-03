/**
 *  @file fsm.h
 *  @brief Header file for the main finite state machine of the BMS
 *
 *  @date 2020/05/02
 *  @author Andrew Hanlon (a2k-hanlon)
 */

#ifndef INC_FSM_H_
#define INC_FSM_H_

/*============================================================================*/
/* ENUMERATIONS */

typedef enum
{
    BMS_RESET = 0
    // BMS_<STATE> ... Add states here
} FSM_state_t;

/*============================================================================*/
/* FUNCTION PROTOTYPES */

void FSM_init(void);
void FSM_run(void);
void FSM_reset(void);
// More state functions go here

#endif /* INC_FSM_H_ */
