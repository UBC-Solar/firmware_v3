/**
 *  @file controls.c
 *  @brief Functions for control signals to the ECU
 *
 *  The control signals are:
 *  BAL  - Balancing active indication
 *  COM  - Communication fault (with slave boards) indication
 *  FLT  - General Fault indication (active for any fault)
 *  GPIO - General Purpose Input Output - not implemented here; future proofing
 *  HLIM - High limit; highest voltage module is over-voltage
 *  LLIM - Low limit; lowest voltage module is under-voltage
 *  OT   - Over temperature indication
 *
 *  FAN_PWM - Pulse width modulated signal for cooling fan control
 *
 *  @date 2020/04/29
 *  @author Andrew Hanlon (a2k-hanlon)
 */

#include <control.h>

void CONT_init()
{
    CONT_BAL_state = CONT_INACTIVE;
    CONT_COM_state = CONT_INACTIVE;
    CONT_FLT_state = CONT_INACTIVE;
    CONT_HLIM_state = CONT_INACTIVE;
    CONT_LLIM_state = CONT_INACTIVE;
    CONT_OT_state = CONT_INACTIVE;

    CONT_BAL_switch(CONT_INACTIVE);
    CONT_COM_switch(CONT_INACTIVE);
    CONT_FLT_switch(CONT_INACTIVE);
    CONT_HLIM_switch(CONT_INACTIVE);
    CONT_LLIM_switch(CONT_INACTIVE);
    CONT_OT_switch(CONT_INACTIVE);

    CONT_FAN_PWM_set(0);
    HAL_TIM_PWM_Start(CONT_timer_handle, CONT_TIM_CHANNEL);
}

void CONT_BAL_switch(CONT_signal_state_t new_state)
{
    HAL_GPIO_WritePin(CONT_BAL_PORT, CONT_BAL_PIN, new_state ^ CONT_BAL_POLARITY);
    CONT_BAL_state = new_state;
}

void CONT_COM_switch(CONT_signal_state_t new_state)
{
    HAL_GPIO_WritePin(CONT_COM_PORT, CONT_COM_PIN, new_state ^ CONT_COM_POLARITY);
    CONT_COM_state = new_state;
}

void CONT_FLT_switch(CONT_signal_state_t new_state)
{
    HAL_GPIO_WritePin(CONT_FLT_PORT, CONT_FLT_PIN, new_state ^ CONT_FLT_POLARITY);
    CONT_FLT_state = new_state;
}

void CONT_HLIM_switch(CONT_signal_state_t new_state)
{
    HAL_GPIO_WritePin(CONT_HLIM_PORT, CONT_HLIM_PIN, new_state ^ CONT_HLIM_POLARITY);
    CONT_HLIM_state = new_state;
}

void CONT_LLIM_switch(CONT_signal_state_t new_state)
{
    HAL_GPIO_WritePin(CONT_LLIM_PORT, CONT_LLIM_PIN, new_state ^ CONT_LLIM_POLARITY);
    CONT_LLIM_state = new_state;
}

void CONT_OT_switch(CONT_signal_state_t new_state)
{
    HAL_GPIO_WritePin(CONT_OT_PORT, CONT_OT_PIN, new_state ^ CONT_OT_POLARITY);
    CONT_OT_state = new_state;
}

/**
 * @brief Sets the PWM duty cycle of the FAN_PWM output.
 *
 * @param pwm_val The duty cycle of the PWM. Must be in range [0,100].
 */
void CONT_FAN_PWM_set(unsigned int pwm_val)
{
    unsigned int CONT_FAN_PWM_percent = pwm_val;
    pwm_val = (pwm_val * PWM_DIVISIONS) / 100;
    __HAL_TIM_SET_COMPARE(CONT_timer_handle, CONT_TIM_CHANNEL, pwm_val);
}
