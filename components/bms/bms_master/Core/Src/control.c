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

#include "control.h"

typedef struct {
    TIM_HandleTypeDef *timer_handle;
    uint32_t timer_pwm_channel;
    unsigned int FAN_PWM_percent;
} CONT_data_t;

static CONT_data_t CONT_data;

void CONT_init(TIM_HandleTypeDef *timer_handle, uint32_t timer_pwm_channel)
{
    CONT_data.timer_handle = timer_handle;
    CONT_data.timer_pwm_channel = timer_pwm_channel;

    CONT_BAL_switch(false);
    CONT_COM_switch(false);
    CONT_FLT_switch(false);          // ECU expects it to be HIGH on startup
    CONT_HLIM_switch(false);
    CONT_LLIM_switch(false);
    CONT_OT_switch(false);

    CONT_FAN_PWM_set(0);
    HAL_TIM_PWM_Start(CONT_data.timer_handle, CONT_data.timer_pwm_channel);
}

void CONT_BAL_switch(bool new_state)
{
    HAL_GPIO_WritePin(CONT_BAL_PORT, CONT_BAL_PIN, new_state ^ CONT_BAL_POLARITY);
}

void CONT_COM_switch(bool new_state)
{
    HAL_GPIO_WritePin(CONT_COM_PORT, CONT_COM_PIN, new_state ^ CONT_COM_POLARITY);
}

void CONT_FLT_switch(bool new_state)
{
    HAL_GPIO_WritePin(CONT_FLT_PORT, CONT_FLT_PIN, new_state ^ CONT_FLT_POLARITY);
}

void CONT_HLIM_switch(bool new_state)
{
    HAL_GPIO_WritePin(CONT_HLIM_PORT, CONT_HLIM_PIN, new_state ^ CONT_HLIM_POLARITY);
}

void CONT_LLIM_switch(bool new_state)
{
    HAL_GPIO_WritePin(CONT_LLIM_PORT, CONT_LLIM_PIN, new_state ^ CONT_LLIM_POLARITY);
}

void CONT_OT_switch(bool new_state)
{
    HAL_GPIO_WritePin(CONT_OT_PORT, CONT_OT_PIN, new_state ^ CONT_OT_POLARITY);
}

/**
 * @brief Sets the PWM duty cycle of the FAN_PWM output.
 *
 * Note that depending on the timer configuration, the duty cycle may not be perfect.
 * Perfect agreement between the integer percentage given and the actual duty cycle
 * output is possible, but not really necessary for driving fans.
 * A "correct" timer setup is at least pretty close (within +-0.5%)
 *
 * @param pwm_val The duty cycle of the PWM. Must be in range [0,100].
 */
void CONT_FAN_PWM_set(unsigned int pwm_val)
{
    CONT_data.FAN_PWM_percent = pwm_val;
    pwm_val = (pwm_val * PWM_DIVISIONS) / 100;
    __HAL_TIM_SET_COMPARE(CONT_data.timer_handle, CONT_data.timer_pwm_channel, pwm_val);
}

/**
 * @brief Calculates a PWM duty cycle percentage for fans based on a temperature
 *
 * The percentage is linearly dependent on temperature. Below FAN_OFF_TEMP, the
 * fans are off. Above FAN_FULL_TEMP, the fans are on full. In between those 2
 * values, the fans ramp linearly with temperature from MIN_FAN_PWM to 100%.
 *
 * To avoid spurious fan startup and shutdown, control with hysteresis
 * is used. If the fans are started, (say, the temp rises above FAN_OFF_TEMP),
 * they won't actually be turned off until the temperature drops to
 * TEMP_HYSTERESIS degrees below FAN_OFF_TEMP.
 *
 * @param[in] temp Temperature to use in calculation
 * @return The duty cycle percentage for fan PWM
 */
unsigned int CONT_fanPwmFromTemp(float temp)
{
    int new_fan_PWM = FAN_RAMP_SLOPE * temp + MIN_FAN_PWM;
    // Limit range to [MIN_FAN_PWM, 100]
    if (new_fan_PWM < MIN_FAN_PWM)
        new_fan_PWM = MIN_FAN_PWM;
    if (new_fan_PWM > 100)
        new_fan_PWM = 100;

    if (CONT_data.FAN_PWM_percent != 0) // If fans are on...
    {
        // Don't turn fans off unless temp is low enough
        if (temp < FAN_OFF_TEMP - TEMP_HYSTERESIS)
        {
            new_fan_PWM = 0;
        }
    }
    else // If fans are off...
    {
        // Don't turn fans on unless temp is high enough
        if (temp < FAN_OFF_TEMP)
        {
            new_fan_PWM = 0;
        }
    }

    return new_fan_PWM; // a cast to unsigned happens here
}
