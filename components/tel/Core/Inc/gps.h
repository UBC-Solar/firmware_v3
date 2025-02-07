/******************************************************************************
* @file    gps.h
* @brief   Stores global GPS variables and GPS functions
******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "stdbool.h"
#include "i2c.h"
#include "main.h"


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GPS__H__
#define __GPS__H__

#define GPS_MESSAGE_LEN 500

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c);

void gps_task();

bool read_i2c_gps_module(uint8_t* receive_buffer);

#endif /* __GPS__H__ */