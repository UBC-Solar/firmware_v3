/******************************************************************************
* @file    gps.h
* @brief   Stores global GPS variables and GPS functions
******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "stdbool.h"
#include "i2c.h"

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GPS__H__
#define __GPS__H__

#define GPS_MESSAGE_LEN 500

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c);

bool read_i2c_gps_module(uint8_t* receive_buffer);

extern bool g_gps_read_okay;

extern uint8_t g_gps_data[GPS_MESSAGE_LEN];
extern char gps_parse_data[GPS_MESSAGE_LEN];


#endif /* __GPS__H__ */