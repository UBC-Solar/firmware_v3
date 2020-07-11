/**
 *  @file control.h
 *  @brief Header file for ECU control signal outputs functionality
 *
 *  Steps to set up this functionality:
 *  1.  Set the GPIO port and pin for all signals in this file
 *  2.  After initializing the STM32's TIM peripheral in the application code,
 *      set the CONT_TIM_handle pointer to the HAL TIM handle
 *      eg. CONT_TIM_handle = &htim3;
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
    CONT_INACTIVE = 0,
    CONT_ACTIVE = 1
} CONT_signal_state_t;

/*============================================================================*/
/* CONFIGURABLE PARAMETERS */

#define CONT_BAL_PORT GPIOB
#define CONT_BAL_PIN GPIO_PIN_12

#define CONT_COM_PORT GPIOB
#define CONT_COM_PIN GPIO_PIN_13

#define CONT_FLT_PORT GPIOB
#define CONT_FLT_PIN GPIO_PIN_14

#define CONT_HLIM_PORT GPIOB
#define CONT_HLIM_PIN GPIO_PIN_15

#define CONT_LLIM_PORT GPIOA
#define CONT_LLIM_PIN GPIO_PIN_8

#define CONT_OT_PORT GPIOA
#define CONT_OT_PIN GPIO_PIN_9

// For all signals, this is the inactive pin state
// For LLIM and HLIM, this is the pin state when contactors should be closed
#define CONT_BAL_POLARITY 0
#define CONT_COM_POLARITY 0
#define CONT_FLT_POLARITY 0
#define CONT_HLIM_POLARITY 0
#define CONT_LLIM_POLARITY 0
#define CONT_OT_POLARITY 0

#define CONT_TIM_CHANNEL TIM_CHANNEL_2 // must match the peripheral config
#define PWM_DIVISIONS 320 // This should be equal to the value of the
// auto-reload register of the timer used for PWM generation + 1

// Parameters for linear temp -> fan PWM calculation
#define FAN_OFF_TEMP    20.0 // degrees C; fans off below this temp, on above, starting at MIN_FAN_PWM
#define FAN_FULL_TEMP   45.0 // Fans will be on full beyond this temp
                             //   (they'll be on full regardless under FAULT)
#define MIN_FAN_PWM     15   // %; Fans may not actually spin at very low duty cycles
#define TEMP_HYSTERESIS 1.0
#define FAN_RAMP_SLOPE  ((100 - MIN_FAN_PWM) / (FAN_FULL_TEMP - FAN_OFF_TEMP))

/*============================================================================*/
/* PUBLIC CONSTANTS */

/*============================================================================*/
/* PUBLIC VARAIBLES */

TIM_HandleTypeDef * CONT_timer_handle;

// For code outside control.c, consider these READ-ONLY
CONT_signal_state_t CONT_BAL_state;
CONT_signal_state_t CONT_COM_state;
CONT_signal_state_t CONT_FLT_state;
CONT_signal_state_t CONT_HLIM_state;
CONT_signal_state_t CONT_LLIM_state;
CONT_signal_state_t CONT_OT_state;
unsigned int CONT_FAN_PWM_percent;

/*============================================================================*/
/* FUNCTION PROTOTYPES */

void CONT_init(void);

void CONT_BAL_switch(CONT_signal_state_t new_state);
void CONT_COM_switch(CONT_signal_state_t new_state);
void CONT_FLT_switch(CONT_signal_state_t new_state);
void CONT_HLIM_switch(CONT_signal_state_t new_state);
void CONT_LLIM_switch(CONT_signal_state_t new_state);
void CONT_OT_switch(CONT_signal_state_t new_state);
void CONT_FAN_PWM_set(unsigned int pwm_val);
unsigned int fanPwmFromTemp(float temp);

#endif /* INC_CONTROL_H_ */

/**
 * Note the following for configuring a timer for PWM:
 * - Use a general purpose timer, not a basic timer.
 * - Slave Mode: Disable
 * - Trigger Source: Disable
 * - Clock Source: Internal Clock
 * - Configure the applicable channel for PWM generation
 * - Autoreload value: greater than 100.
 * - Prescaler: set in conjunction with the autoreload value so that
 *              the PWM frequency is 25kHz.
 *              Prescaler = Clock_Freq / ( 25000 * (autreload + 1)) - 1
 * eg. for 8 MHz clock, could do autorelaod = 320 and a prescaler of 0
 * - Upcounting timer
 * - No internal clock division
 * - auto-reload preload is not needed
 * - PWM mode 1
 * - Configure the pulse to 0; it will be set by CONT_FAN_PWM_set()
 * - output compare preload enabled
 * - CH polarity can be used to change the polarity like the
 *   POLARITY constants above
 */
