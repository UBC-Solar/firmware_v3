/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GPS__H__
#define __GPS__H__
#include "stdbool.h"

#define GPS_MESSAGE_LEN 1000

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c);

extern bool g_gps_read_okay;

#endif /* __GPS__H__ */