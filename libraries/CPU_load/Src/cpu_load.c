/*
 * cpu_load.c
 *
 *  Created on: Nov 9, 2024
 *      Author: diego
 */



#include "cpu_load.h"
#include "circular_buffer.h"

#define FLOAT_TO_PERCENTAGE 100.0f

static uint16_t idleRunTime = 0;
static uint16_t idle_switched_in = 0;
static uint16_t window_start_time = 0;
static uint16_t total_run_time = 0;
static float cpu_load = 0;

static uint8_t g_window_size;
static uint16_t g_frequency_ms;
static TIM_HandleTypeDef g_timer_handle;

static float cpu_load_buffer[MAX_CPU_LOAD_ENTRIES];

void CPU_monitor_task(void* argument);

typedef StaticTask_t osStaticThreadDef_t;

/* Definitions for CPU monitor task */
osThreadId_t CPU_monitor_task_handle;
uint32_t CPU_monitor_task_buffer[ 128 ];
osStaticThreadDef_t CPU_taskControlBlock;
const osThreadAttr_t CPU_Task_attributes = {
  .name = "CPU_monitor_task",
  .cb_mem = &CPU_taskControlBlock,
  .cb_size = sizeof(CPU_taskControlBlock),
  .stack_mem = &CPU_monitor_task_buffer[0],
  .stack_size = sizeof(CPU_monitor_task_buffer),
  .priority = (osPriority_t) osPriorityRealtime7,
};

circ_buf_t cpuLoadBuffer = {
    .pBuffer = cpu_load_buffer,
    .head = 0,
    .tail = 0,
    .num_entries = 0,
};

void taskSwitchedIn(){
	TaskHandle_t idleTaskHandle = xTaskGetIdleTaskHandle();
	if(xTaskGetCurrentTaskHandle() == idleTaskHandle){
		idle_switched_in = __HAL_TIM_GET_COUNTER(&htim10);
	}
}

void taskSwitchedOut(){
	TaskHandle_t idleTaskHandle = xTaskGetIdleTaskHandle();
	if(xTaskGetCurrentTaskHandle() == idleTaskHandle){
		idleRunTime += __HAL_TIM_GET_COUNTER(&htim10) - idle_switched_in;
	}
}

void reset(){
	idleRunTime = 0;
	window_start_time = __HAL_TIM_GET_COUNTER(&htim10);
}

float CPU_LOAD_calculation(float idleRunTime, float total_run_time){
	return (1.0f - ((float)idleRunTime / total_run_time)) * FLOAT_TO_PERCENTAGE;
}


float CPU_LOAD_init(uint8_t window_size, uint16_t frequency_ms, TIM_HandleTypeDef *timer){

    g_window_size = window_size;
    g_frequency_ms = frequency_ms;
    g_timer_handle = *timer;

    HAL_TIM_Base_Start(&g_timer_handle);

    CPU_monitor_task_handle = osThreadNew(CPU_monitor_task, NULL, &CPU_Task_attributes)
}

void add_to_buffer(){

	total_run_time = __HAL_TIM_GET_COUNTER(&htim10) - window_start_time;

    float cpu_load = CPU_LOAD_calculation((float)idleRunTime, (float)total_run_time);
    CIRC_BUF_enqueue(&cpuLoadBuffer, cpu_load, g_window_size);
    reset();
	
}

float CPU_LOAD_average() {
    if (CIRC_BUF_empty(&cpuLoadBuffer)) {
        return -1;  // Return an indicator for no data in buffer
    }

    float sum = 0.0f;
    for (int i = 0; i < cpuLoadBuffer.num_entries; i++) {
        sum += cpuLoadBuffer.pBuffer[(cpuLoadBuffer.tail + i) % MAX_CPU_LOAD_ENTRIES];
    }

    return sum / cpuLoadBuffer.num_entries;
}

void CPU_monitor_task(void* argument){

    for(;;){
        add_to_buffer();
        cpu_load = CPU_LOAD_average();
        osDelay(g_frequency_ms);
    }
}