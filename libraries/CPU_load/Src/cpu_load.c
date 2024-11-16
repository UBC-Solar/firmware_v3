#include "cpu_load.h"
#include "circular_buffer.h"

#define FLOAT_TO_PERCENTAGE 100.0f
#define UINT16_NUM_BITS     16

static uint64_t idle_run_time = 0;
static uint64_t idle_switched_in = 0;
static uint64_t window_start_time = 0;
static uint64_t total_run_time = 0;
static uint64_t overflow_count = 0;

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



/**
 * @brief Handler for timer overflow
 *
 * This gets called in the period elapsed function to check if it
 * is the timer being passed to the CPU Load software component that has
 * overflown. When it has, it increments the overflow_count global variable
 * which is used to keep track of time accurately even when the timer
 * has overflown. 
 *
 * @param htim A pointer to a timer handler
 */

void CPU_LOAD_timer_overflow_handler(TIM_HandleTypeDef *htim) {
    if (htim->Instance == g_timer_handle.Instance) {
        overflow_count++;
    }
}

/**
 * @brief Trace hook macro for switching into a task
 *
 * This function gets called when a task has been switched into. It
 * then checks if the task being switched into is the idle task.
 * If it is the idle task, we take note of the time we entered it 
 * by combining the current time and overflown time into a global 
 * variable called idle_switched_in. 
 */

void taskSwitchedIn() {
    TaskHandle_t idleTaskHandle = xTaskGetIdleTaskHandle();
    if (xTaskGetCurrentTaskHandle() == idleTaskHandle) {
        uint16_t timer_value = __HAL_TIM_GET_COUNTER(&g_timer_handle);
        idle_switched_in = (overflow_count << UINT16_NUM_BITS) | timer_value;
    }
}

/**
 * @brief Trace hook macro for switching out of a task
 *
 * This function gets called when a task has been switched out of. If the
 * task being switched out of was the idle task, then we do calculations 
 * involving the current time, overflow time, and the time we switched into 
 * the idle task, to determine how long we spent in the idle task.
 */

void taskSwitchedOut() {
    TaskHandle_t idleTaskHandle = xTaskGetIdleTaskHandle();
    if (xTaskGetCurrentTaskHandle() == idleTaskHandle) {
        uint16_t timer_value = __HAL_TIM_GET_COUNTER(&g_timer_handle);
        uint64_t current_time = (overflow_count << UINT16_NUM_BITS) | timer_value;
        idle_run_time += current_time - idle_switched_in;
    }
}

/**
 * @brief Reset for new window of CPU Load calculation
 *
 * This function resets the idle_run_time to 0 and takes note of the new
 * window's start time in window_start_time, in order to calculate a new CPU Load. This function
 * gets called only after we have calculated and added a CPU Load to the circular
 * buffer.
 */

void reset() {
    idle_run_time = 0;
    uint16_t timer_value = __HAL_TIM_GET_COUNTER(&g_timer_handle);
    window_start_time = (overflow_count << UINT16_NUM_BITS) | timer_value;
}

/**
 * @brief Calculates the CPU Load
 *
 * This function is the actual formula for calculating a CPU Load. 
 * It simply takes a ratio of the idle_run_time and total_run_time and 
 * multiplies by 100 to get a percentage. You can think of this as a ratio
 * of free-time over busy-time. So a CPU Load of 100% means there was no free-time.
 *
 * @param idle_run_time The time spent in the idle task for this window
 * @param total_run_time The total time in this window
 * @return The CPU Load as a percentage
 */

float CPU_LOAD_calculation(float idle_run_time, float total_run_time) {
    return (1.0f - ((float)idle_run_time / total_run_time)) * FLOAT_TO_PERCENTAGE;
}

/**
 * @brief Necessary initializations for the software component
 *
 * This function takes and stores the user's desired window size, 
 * calculation frequency, and timer in global variables so that the 
 * rest of the software component has access to these desired constraints.
 * This function also starts the given timer and creates the RTOS task.
 *
 * @param window_size The desired amount of items in the circular buffer
 * @param frequency_ms The frequency for a calculation in milliseconds
 * @param timer A pointer to a timer handler
 */

void CPU_LOAD_init(uint8_t window_size, uint16_t frequency_ms, TIM_HandleTypeDef *timer) {
    g_window_size = window_size;
    g_frequency_ms = frequency_ms;
    g_timer_handle = *timer;

    HAL_TIM_Base_Start_IT(&g_timer_handle);  // Start timer with interrupts enabled for overflow handling

    CPU_monitor_task_handle = osThreadNew(CPU_monitor_task, NULL, &CPU_Task_attributes);
}

/**
 * @brief Adds a CPU Load to the circular buffer
 *
 * This function takes the note of the total time passed and stores it in
 * total_run_time. The CPU_LOAD_calculation then gets called with total_run_time
 * and idle_run_time being passed in and the return value is stored in cpu_load.
 * This CPU load is then added to the circular buffer with CIRC_BUF_enqueue and the reset
 * function gets called so as to indicate a new window start. 
 */

void add_to_buffer() {
    uint16_t timer_value = __HAL_TIM_GET_COUNTER(&g_timer_handle);
    total_run_time = ((overflow_count << UINT16_NUM_BITS) | timer_value) - window_start_time;

    float cpu_load = CPU_LOAD_calculation((float)idle_run_time, (float)total_run_time);
    CIRC_BUF_enqueue(&cpuLoadBuffer, cpu_load, g_window_size);
    reset();
}

/**
 * @brief Takes the average of the CPU loads in the buffer
 *
 * This function first checks to make sure the buffer isn't empty,
 * then iterates thru the circular buffer and takes an average of all the CPU Loads 
 * so as to provide the user with a more accurate CPU load over time. 
 *
 * @return An average of the CPU loads as a percentage.
 */

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

/**
 * @brief FreeRTOS task for monitoring the CPU load
 *
 * This task is the highest priority task only second to all interrupts.
 * It adds CPU loads to the buffer by calling the add_to_buffer function and calls
 * an osDelay for the amount of time that the user specified in their given frequency.
 *
 * @param argument Unused
 */

void CPU_monitor_task(void* argument) {
    for (;;) {
    	HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_1);
        add_to_buffer();
        osDelay(g_frequency_ms);
    }
}
