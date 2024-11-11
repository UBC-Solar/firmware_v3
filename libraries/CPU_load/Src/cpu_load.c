/*
 * cpu_load.c
 *
 *  Created on: Nov 9, 2024
 *      Author: diego
 */

#include "cpu_load.h"
#include "circular_buffer.h"

#define FLOAT_TO_PERCENTAGE 100.0f

static uint64_t idleRunTime = 0;
static uint64_t idle_switched_in = 0;
static uint64_t window_start_time = 0;
static uint64_t total_run_time = 0;
static uint64_t overflow_count = 0;
static float cpu_load = 0;

static uint8_t g_window_size;
static uint16_t g_frequency_ms;
static TIM_HandleTypeDef g_timer_handle;

static float cpu_load_buffer[MAX_CPU_LOAD_ENTRIES];

void CPU_monitor_task(void* argument);

typedef StaticTask_t osStaticThreadDef_t;

/* Definitions for CPU monitor task */
osThreadId_t CPU_monitor_task_handle;
uint32_t CPU_monitor_task_buffer[128];
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

// Timer overflow handler
void CPU_LOAD_timer_overflow_handler(TIM_HandleTypeDef *htim) {
    if (htim->Instance == g_timer_handle.Instance) {
        overflow_count++;
    }
}

// Task switched in - capture the starting time
void taskSwitchedIn() {
    TaskHandle_t idleTaskHandle = xTaskGetIdleTaskHandle();
    if (xTaskGetCurrentTaskHandle() == idleTaskHandle) {
        uint16_t timer_value = __HAL_TIM_GET_COUNTER(&g_timer_handle);
        idle_switched_in = (overflow_count << 16) | timer_value;
    }
}

// Task switched out - calculate idle time based on current time
void taskSwitchedOut() {
    TaskHandle_t idleTaskHandle = xTaskGetIdleTaskHandle();
    if (xTaskGetCurrentTaskHandle() == idleTaskHandle) {
        uint16_t timer_value = __HAL_TIM_GET_COUNTER(&g_timer_handle);
        uint64_t current_time = (overflow_count << 16) | timer_value;
        idleRunTime += current_time - idle_switched_in;
    }
}

// Reset the counters for a new monitoring window
void reset() {
    idleRunTime = 0;
    uint16_t timer_value = __HAL_TIM_GET_COUNTER(&g_timer_handle);
    window_start_time = (overflow_count << 16) | timer_value;
}

// Calculate CPU load percentage
float CPU_LOAD_calculation(float idleRunTime, float total_run_time) {
    return (1.0f - ((float)idleRunTime / total_run_time)) * FLOAT_TO_PERCENTAGE;
}

// Initialize the CPU load monitoring
void CPU_LOAD_init(uint8_t window_size, uint16_t frequency_ms, TIM_HandleTypeDef *timer) {
    g_window_size = window_size;
    g_frequency_ms = frequency_ms;
    g_timer_handle = *timer;

    HAL_TIM_Base_Start_IT(&g_timer_handle);  // Start timer with interrupts enabled for overflow handling

    CPU_monitor_task_handle = osThreadNew(CPU_monitor_task, NULL, &CPU_Task_attributes);
}

// Add CPU load data to the circular buffer and reset for the next window
void add_to_buffer() {
    uint16_t timer_value = __HAL_TIM_GET_COUNTER(&g_timer_handle);
    total_run_time = ((overflow_count << 16) | timer_value) - window_start_time;

    float cpu_load = CPU_LOAD_calculation((float)idleRunTime, (float)total_run_time);
    CIRC_BUF_enqueue(&cpuLoadBuffer, cpu_load, g_window_size);
    reset();
}

// Calculate the average CPU load from the buffer
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

// CPU monitoring task - periodically calculates and stores CPU load
void CPU_monitor_task(void* argument) {
    for (;;) {
        add_to_buffer();
        cpu_load = CPU_LOAD_average();
        osDelay(g_frequency_ms);
    }
}
