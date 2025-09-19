/**
 *  @file balancing.h
 *  @brief Header file for module passive balancing algorithm
 *
 *  @date 2020/08/18
 *  @author abooodeee
 */

#ifndef INC_BALANCING_H_
#define INC_BALANCING_H_

#include "pack.h"

/*============================================================================*/
/* CONFIGURABLE PARAMETERS */

// Threshold module voltage above which to perform balancing
// At least one module must have a voltage higher than this threshold for balancing to take place.

/**
 * Note: 
 * At comp: Set constant to 4.0V
 * Otherwsise set to 2.9V
 * See  https://ubcsolar26.monday.com/boards/7524367629/pulses/7524367947/posts/3569860014 for explanation
 */
#define BAL_MIN_VOLTAGE_FOR_BALANCING 29000U // 4.0V

// Voltage difference within which modules are counted as balanced
#define BAL_VOLTAGE_DIFFERENCE_TOLERANCE 300U // 0.03V. 

// Voltages are multiplied by 10^4 to match the format in the pack

/*============================================================================*/
/* FUNCTION PROTOTYPES */

void BAL_updateBalancing(Pack_t *pack);

#endif /* INC_BALANCING_H_ */
