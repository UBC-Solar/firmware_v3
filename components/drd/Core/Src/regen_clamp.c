/*
 * drive_state.c
 *
 *	@brief	Handles the drive state for the car. Takes ADC, GPIO, and CAN Inputs, and outputs a DAC values
 *		as well as flags for the MDI.
 *
 *  Created on: June 06, 2025
 *      Author: Tony Chen and Mridul Singh
 *
 */

 /*	Includes	*/
#include "drive_state.h"
#include "adc.h"
#include "CAN_comms.h"
#include <string.h>
#include <stdlib.h>
#include "diagnostic.h"
#include "cyclic_data_handler.h"
#include "regen_clamp.h"
#include <math.h>

#define PACK_CURRENT_NEGATIVE_UNCERTAINTY       (0.2f)

/* STATE */
typedef enum { RC_IDLE, RC_HARD_CAP, RC_DYNAMIC } rc_state_t;

static rc_state_t s_state = RC_IDLE;
static float s_prev_regen_A = 0.0f;
static float s_cap_factor = REGEN_HARD_CAP_FACTOR;

/* Helpers - clamps a float v to stay within a range [lo, hi]*/
static inline float clampf(float v, float lo, float hi)
{ return fminf(hi, fmaxf(lo, v)); }

/* Setting regen clamp */
uint16_t RegenClamp_get_regen_dac(uint16_t  throttle_dac,
                                float pack_current, float array_current)
{
    /* 1.  If driver doesn’t want regen or pedal is > regen zone then reset. */
    if (throttle_dac > THROTTLE_REGEN_ZONE_DAC)
    {
        RegenClamp_reset(); 
        return 0;
    }

    /* 2.  We find the instantaneous regen component (positive magnitude). */
    // TODO: Add in LVS current.
    float regen_A = -(pack_current - array_current);   // Because pack current is always higher
    if (regen_A < PACK_CURRENT_NEGATIVE_UNCERTAINTY) regen_A = 0.0f;                           // For safety

    /* 3.  State-machine */
    switch (s_state)
    {
        /* First frame after entry: hard-cap*/
        case RC_IDLE:
            s_state        = RC_HARD_CAP;
            s_cap_factor   = REGEN_HARD_CAP_FACTOR;
            s_prev_regen_A = regen_A;
            break;

        /* Stay at hard-cap until regen drops once (spike ended) */
        case RC_HARD_CAP:
            if (regen_A <= s_prev_regen_A)            // first downward datapoint. Justified because regen spikes up wildly
            {
                /* Find uncapped peak regen current. */
                float uncapped_peak_regen_A = regen_A / REGEN_HARD_CAP_FACTOR; 

                /* Solve:  TARGET = uncapped_peak_regen * x  + array_current   →   x = …  */
                float x = (MAX_PACK_CURRENT - fabsf(array_current)) / uncapped_peak_regen_A;
                s_cap_factor = clampf(x, 0.0f, 1.0f);

                s_state = RC_DYNAMIC;
            }
            s_prev_regen_A = regen_A;
            break;

        /* -- Dynamic phase: just hold the computed factor ---------------- */
        case RC_DYNAMIC:
            // TODO make factor 100% at one point
            // TODO go back to IDLE state at some point
            if (regen_A <= PACK_CURRENT_NEGATIVE_UNCERTAINTY){
                s_state = RC_IDLE;
            }
            break;
        default:
            /* Nothing to do; factor frozen until regen ends/reset() */
            break;
    }

    /* 4.  Return DAC value */
    uint16_t dac = (uint16_t)(s_cap_factor * 1023.0f + 0.5f);   // round-to-nearest
    return dac;
}

void RegenClamp_reset(void)
{
    s_state          = RC_IDLE;
    s_cap_factor     = REGEN_HARD_CAP_FACTOR;
    s_prev_regen_A   = 0.0f;
}