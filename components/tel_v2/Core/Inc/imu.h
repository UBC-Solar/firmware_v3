#include <stdint.h>

#define IMU_MESSAGE_LEN 17

#define TIMESTAMP_BYTE(i, timestamp) ((timestamp >> (i * 8)) & 0xFF);

typedef struct {
  uint8_t imu_type;
  uint8_t dimension;
  uint8_t data[4];
} IMU_msg_t;


void transmit_imu_data(time_t current_timestamp, uint8_t* imu_data, char imu_type, char dimension);
