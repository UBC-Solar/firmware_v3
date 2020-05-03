/**
 *  @file fsm.c
 *  @brief Main finite state machine for the BMS
 *
 *  @date 2020/05/02
 *  @author Andrew Hanlon (a2k-hanlon)
 */

/*
 * Note: Probably the most common implementation of an FSM in C uses
 * a switch statement with a case for each state. This is perfectly
 * valid, but that's not what is done here.
 * What's done here uses a function pointer array (FSM_state_table)
 * along with a function for every state. For longer and more complicated
 * FSMs, this style of FSM is beneficial as it separates the states
 * more distinctly.
 */

#include "fsm.h"

// Function pointer array to BMS FSM state functions
// Function names go in this array declaration, and must be in the same order
// as the FSM_BMS_state_t enumeration.
void (* FSM_state_table[])() = {FSM_reset};

FSM_state_t FSM_state;

void FSM_init()
{
    FSM_state = BMS_RESET;
}

// This function should be placed in the main firmware loop
void FSM_run()
{
    FSM_state_table[FSM_state]();
}

void FSM_reset()
{
    // Loop unto self until more states are implemented
    FSM_state = BMS_RESET; // TODO: Change once other states are in place
}

/*
 * State functions should follow this format:
 * void FSM_<state name>()
 * {
 *      <Things to do in this state no matter what>
 *
 *      if(<state change condition>)
 *      {
 *          <Things to do on state transition>
 *          FSM_state = <state to go to on next loop>;
 *      }
 *      else if (<other state change condition>)
 *      {
 *          <Things to do on state transition>
 *          FSM_state = <state to go to on next loop>;
 *      }
 *      // staying in the same state otherwise can be implicit.
 * }
 */
