
/**
 *  @file rtos_tasks.h
 *  @brief Defines task priorities, stack sizes, delays, and other task parameters to register an RTOS task.
 *
 *  @date 2023/03/18
 *  @author Aarjav Jain
 */


/* Size Constants */
#define MAX_CAN_MSGS_IN_POOL                        32              // Maximum number of CAN messages in the CAN message pool 

/* TASK DELAYS */
#define DEFAULT_TASK_DELAY                          100             // 100ms
#define TRANSMIT_DIAGNOSTICS_DELAY                  2000            // 2000ms
#define GPS_WAIT_MSG_DELAY                          10 * 1000       // 10 seconds/10000ms wait before checking for GPS msg
#define IMU_SINGLE_DELAY                            125             // 125ms single delay, * 2 = 250ms per group of IMU messages.

/* TASK INSTANCES */
#define RTOS_TASKS_INSTANCES                        0              
