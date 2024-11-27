/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GPS__H__
#define __GPS__H__
#include "stdbool.h"
#include "i2c.h"

#define GPS_MESSAGE_LEN 300

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c);

bool read_i2c_gps_module(uint8_t* receive_buffer);

extern bool g_gps_read_okay;

#endif /* __GPS__H__ */