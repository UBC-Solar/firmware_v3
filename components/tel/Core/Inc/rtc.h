/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    rtc.h
  * @brief   This file contains all the function prototypes for
  *          the rtc.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RTC_H__
#define __RTC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern RTC_HandleTypeDef hrtc;

/* USER CODE BEGIN Private defines */

#define MILLISECONDS_TO_SECONDS         0.001
#define MILLISECONDS_IN_SECONDS         1000

#define TIMETYPEDEF_SECONDS_IDX         0
#define TIMETYPEDEF_MINUTES_IDX         1
#define TIMETYPEDEF_HOURS_IDX           2
#define TIMETYPEDEF_DAY_IDX             3
#define TIMETYPEDEF_MONTH_IDX           4   
#define TIMETYPEDEF_YEAR_IDX            5

#define RTC_TIMESTAMP_MSG_ID            0x300

/* USER CODE END Private defines */

void MX_RTC_Init(void);

/* USER CODE BEGIN Prototypes */

double RTC_get_timestamp_secs();
void RTC_check_and_sync_rtc(uint32_t can_id, uint8_t* rtc_can_msg_data);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __RTC_H__ */

