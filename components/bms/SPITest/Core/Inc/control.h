/**
 *  @file contactors.h
 *  @brief Header file for contactor control
 *
 *  The 4 contactors are:
 *  NEG  - Negative terminal; the main contactor switching the battery
 *  PCH  - Pre-charge; in parallel with LLIM, for inrush current limiting
 *           at startup
 *  HLIM - High limit; between battery and solar array input
 *  LLIM - Low limit; between batteries and motor controller output
 *
 *  @date 2020/04/29
 *  @author Andrew Hanlon (a2k-hanlon)
 */

#ifndef INC_CONTROL_H_
#define INC_CONTROL_H_

#include "stm32f3xx_hal.h"

/*============================================================================*/
/* ENUMERATIONS */

typedef enum {
    CONT_OPEN = 0,
    CONT_CLOSED = 1
} CONT_contactor_state_t;

/*============================================================================*/
/* CONFIGURABLE PARAMETERS */

#define CONT_NEG_PORT GPIOA
#define CONT_NEG_PIN 0

#define CONT_PCH_PORT GPIOA
#define CONT_PCH_PIN 0

#define CONT_HLIM_PORT GPIOA
#define CONT_HLIM_PIN 0

#define CONT_LLIM_PORT GPIOA
#define CONT_LLIM_PIN 0

// Contactor polarities - the pin state when contactors are open
#define CONT_NEG_POLARITY 0
#define CONT_PCH_POLARITY 0
#define CONT_HLIM_POLARITY 0
#define CONT_LLIM_POLARITY 0

/*============================================================================*/
/* PUBLIC CONSTANTS */

/*============================================================================*/
/* PUBLIC VARAIBLES */

// Do not write to these in external code!
CONT_contactor_state_t CONT_NEG_state;
CONT_contactor_state_t CONT_PCH_state;
CONT_contactor_state_t CONT_HLIM_state;
CONT_contactor_state_t CONT_LLIM_state;

/*============================================================================*/
/* FUNCTION PROTOTYPES */

void CONT_init(void);
void CONT_NEG_switch(CONT_contactor_state_t new_state);
void CONT_PCH_switch(CONT_contactor_state_t new_state);
void CONT_HLIM_switch(CONT_contactor_state_t new_state);
void CONT_LLIM_switch(CONT_contactor_state_t new_state);

#endif /* INC_CONTROL_H_ */
