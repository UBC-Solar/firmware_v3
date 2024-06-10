/**
 *  @file gps.h
 *  @brief header file for gps.c. Define buffer lengths
 *
 *  @date 2023/03/18
 *  @author Diego Armstrong
 */

#include "radio_transmit.h"
#include "i2c.h"
#include "nmea_parse.h"
#include "rtc.h"
#include "rtos_tasks.h"

#define DOUBLE_LAST_BYTE_IDX                7
#define FLOAT_LAST_BYTE_IDX                 3
#define LATSIDE_MSG_IDX                     0
#define LONSIDE_MSG_IDX                     1
#define GPS_RCV_BUFFER_SIZE                 512             // 512 bytes to receive GPS data from I2C
#define UNINITIALIZED                       0
#define INDEX_6                             6
#define INDEX_7                             7
#define NUM_BYTES_FLOAT                     4
#define GPS_SIDE_AND_COUNT_DATA_END_INDEX   5
#define GOT_FIX                             1
#define GPS_SINGLE_MSG_DELAY                2000            // 2 second delay
#define GPS_NEXT_MSG_DELAY                  4000            // 4 second delay

void  GPS_delayed_rx_and_tx_as_CAN();
