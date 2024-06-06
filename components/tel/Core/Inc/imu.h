#include <stdint.h>
#include <time.h>
#include "i2c.h"
#include "usart.h"
#include "sd_logger.h"
#include "rtos_tasks.h"
#include "rtc.h"

#define IMU_MESSAGE_LEN             17
#define ACCEL_SENSITIVITY           16384.0
#define GYRO_SENSITIVITY            131.0
#define MEM_ADDRESS_SIZE            1
#define NUM_BITS_IN_BYTE_SHIFT      8
#define RAW_X_START_BYTE            0
#define RAW_Y_START_BYTE            2
#define RAW_Z_START_BYTE            4  
#define GYRO_DATA_START_BYTE        4

typedef struct {
  uint8_t imu_type;
  uint8_t dimension;
  uint8_t data[4];
} IMU_msg_t;

void IMU_send_as_CAN_msg_with_delay();
