#ifndef CPU_LOAD_INC_CPU_LOAD_H_
#define CPU_LOAD_INC_CPU_LOAD_H_

#include "FreeRTOS.h"
#include "task.h"
#include "stm32f1xx.h"
#include <stdint.h>
#include "cmsis_os2.h"
#include "tim.h"

void CPU_LOAD_init(uint8_t window_size, uint16_t frequency_ms, TIM_HandleTypeDef *timer);
void CPU_LOAD_timer_overflow_handler(TIM_HandleTypeDef *htim);
float CPU_LOAD_average();
#endif /* CPU_LOAD_INC_CPU_LOAD_H_ */
