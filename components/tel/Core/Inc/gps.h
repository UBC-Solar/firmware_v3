/******************************************************************************
* @file    gps.h
* @brief   Stores global GPS variables and GPS functions
******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GPS__H__
#define __GPS__H__

/* Includes ------------------------------------------------------------------*/
#include "stdbool.h"
#include "i2c.h"
#include "main.h"

#define GPS_MESSAGE_LEN 500
#define GPS_TASK_DELAY      250
#define GPS_TASK_OFFSET_DELAY       66

#define GPS_DATA_SAT_COUNT_VIEW_FIX_SNR_CAN_MESSAGE_ID 0x755
#define GPS_DATA_LON_LAT_CAN_MESSAGE_ID                0x756
#define GPS_DATA_HDOP_VDOP_CAN_MESSAGE_ID              0x757
#define GPS_DATA_LON_SIDE_DATE_CAN_MESSAGE_ID          0x758
#define GPS_DATA_PDOP_SPEEDKMH_CAN_MESSAGE_ID          0x759
#define GPS_DATA_ALT_GEOD_CAN_MESSAGE_ID               0x760
#define GPS_DATA_TRUE_MAG_HEADING_CAN_MESSAGE_ID       0x761

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c);

void gps_task();

void read_i2c_gps_module(uint8_t* receive_buffer);

#endif /* __GPS__H__ */
