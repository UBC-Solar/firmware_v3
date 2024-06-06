
/**
 *  @file rtos_tasks.h
 *  @brief Defines task priorities, stack sizes, delays, and other task parameters to register an RTOS task.
 *
 *  @date 2023/03/18
 *  @author Aarjav Jain
 */


/* TASK DELAYS */
#define DEFAULT_TASK_DELAY                          100             // 100ms
#define TRANSMIT_DIAGNOSTICS_DELAY                  2000            // 2000ms
#define GPS_WAIT_MSG_DELAY                          10 * 1000       // 10 seconds/10000ms wait before checking for GPS msg
#define IMU_SINGLE_DELAY                            125             // 125ms single delay, * 2 = 250ms per group of IMU messages.

/* TASK STACK SIZES */
#define RTOS_TASKS_DEFAULT_TASK_STACKSZ             128
#define RTOS_TASKS_READ_CAN_TASK_STACKSZ            512
#define RTOS_TASKS_READ_IMU_TASK_STACKSZ            512
#define RTOS_TASKS_READ_GPS_TASK_STACKSZ            1536
#define RTOS_TASKS_READ_DIAGNOSTIC_TASK_STACKSZ     512

/* TASK PRIORITIES */
#define RTOS_TASKS_DEFAULT_TASK_PRIO                osPriorityLow
#define RTOS_TASKS_READ_CAN_TASK_PRIO               osPriorityNormal
#define RTOS_TASKS_READ_IMU_TASK_PRIO               osPriorityNormal
#define RTOS_TASKS_READ_GPS_TASK_PRIO               osPriorityNormal
#define RTOS_TASKS_READ_DIAGNOSTIC_TASK_PRIO        osPriorityNormal

/* TASK INSTANCES */
#define RTOS_TASKS_INSTANCES                        0
