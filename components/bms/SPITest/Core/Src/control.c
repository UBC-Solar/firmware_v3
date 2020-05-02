/**
 *  @file contactors.c
 *  @brief Functions for controlling the 4 contactors in the battery
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

#include <control.h>

void CONT_init()
{
    CONT_NEG_state = CONT_OPEN;
    CONT_PCH_state = CONT_OPEN;
    CONT_HLIM_state = CONT_OPEN;
    CONT_LLIM_state = CONT_OPEN;

    HAL_GPIO_WritePin(CONT_NEG_PORT, CONT_NEG_PIN, CONT_OPEN ^ CONT_NEG_POLARITY);
    HAL_GPIO_WritePin(CONT_PCH_PORT, CONT_PCH_PIN, CONT_OPEN ^ CONT_PCH_POLARITY);
    HAL_GPIO_WritePin(CONT_HLIM_PORT, CONT_HLIM_PIN, CONT_OPEN ^ CONT_HLIM_POLARITY);
    HAL_GPIO_WritePin(CONT_LLIM_PORT, CONT_LLIM_PIN, CONT_OPEN ^ CONT_LLIM_POLARITY);
}

void CONT_NEG_switch(CONT_contactor_state_t new_state)
{
    HAL_GPIO_WritePin(CONT_NEG_PORT, CONT_NEG_PIN, new_state ^ CONT_NEG_POLARITY);
    CONT_NEG_state = new_state;
}

void CONT_PCH_switch(CONT_contactor_state_t new_state)
{
    HAL_GPIO_WritePin(CONT_PCH_PORT, CONT_PCH_PIN, new_state ^ CONT_PCH_POLARITY);
    CONT_PCH_state = new_state;
}

void CONT_HLIM_switch(CONT_contactor_state_t new_state)
{
    HAL_GPIO_WritePin(CONT_HLIM_PORT, CONT_HLIM_PIN, new_state ^ CONT_HLIM_POLARITY);
    CONT_HLIM_state = new_state;
}

void CONT_LLIM_switch(CONT_contactor_state_t new_state)
{
    HAL_GPIO_WritePin(CONT_LLIM_PORT, CONT_LLIM_PIN, new_state ^ CONT_LLIM_POLARITY);
    CONT_LLIM_state = new_state;
}
