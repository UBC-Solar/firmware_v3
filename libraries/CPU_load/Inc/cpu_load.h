/*
 * cpu_load.h
 *
 *  Created on: Nov 9, 2024
 *      Author: diego
 */

#ifndef CPU_LOAD_INC_CPU_LOAD_H_
#define CPU_LOAD_INC_CPU_LOAD_H_

#include "FreeRTOS.h"
#include "task.h"
#include "stm32f4xx.h"
#include <stdint.h>

float CPU_LOAD_init(uint8_t window_size, uint8_t frequency_ms, TIM_HandleTypeDef *timer);

#endif /* CPU_LOAD_INC_CPU_LOAD_H_ */
