/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    rtc.c
  * @brief   This file provides code for the configuration
  *          of the RTC instances.
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
/* Includes ------------------------------------------------------------------*/
#include "rtc.h"

/* USER CODE BEGIN 0 */

/* INCLUDES */
#include <time.h>
#include <stdbool.h>

/* GLOBALS*/
static bool rtc_set = false;            // TODO: Add diagnostic
static uint32_t start_of_second = 0;    // TODO: Make cleaner timestamp fix

/* USER CODE END 0 */

RTC_HandleTypeDef hrtc;

/* RTC init function */
void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef DateToUpdate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
  hrtc.Init.OutPut = RTC_OUTPUTSOURCE_ALARM;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;

  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  DateToUpdate.WeekDay = RTC_WEEKDAY_MONDAY;
  DateToUpdate.Month = RTC_MONTH_JANUARY;
  DateToUpdate.Date = 0x1;
  DateToUpdate.Year = 0x0;

  if (HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

void HAL_RTC_MspInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspInit 0 */

  /* USER CODE END RTC_MspInit 0 */
    HAL_PWR_EnableBkUpAccess();
    /* Enable BKP CLK enable for backup registers */
    __HAL_RCC_BKP_CLK_ENABLE();
    /* RTC clock enable */
    __HAL_RCC_RTC_ENABLE();
  /* USER CODE BEGIN RTC_MspInit 1 */

  /* USER CODE END RTC_MspInit 1 */
  }
}

void HAL_RTC_MspDeInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspDeInit 0 */

  /* USER CODE END RTC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_RTC_DISABLE();
  /* USER CODE BEGIN RTC_MspDeInit 1 */

  /* USER CODE END RTC_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/**
 * @brief Get the current timestamp in seconds with milliseconds precision
 * 
 * @return double The current timestamp in seconds.
 */
double RTC_get_timestamp_secs()
    {
    RTC_TimeTypeDef time;
    RTC_DateTypeDef date;
    struct tm t;

    HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);              /* RTC hours, mins, seconds */
    HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);              /* RTC years, months, days */

    t.tm_year = date.Year + 100;  // Year since 1900, so add 100 to the year from the RTC (which starts at 2000).
    t.tm_mon = date.Month - 1;    // tm_mon is 0-11, so subtract 1 from the RTC month (which starts at 1).
    t.tm_mday = date.Date;
    t.tm_hour = time.Hours;
    t.tm_min = time.Minutes;
    t.tm_sec = time.Seconds;
    t.tm_isdst = 0;                // Disable daylight saving time adjustments.
    long int epoch_secs = (long int) mktime(&t);

    double milliseconds = MILLIS_TO_SECONDS(((HAL_GetTick() - start_of_second) % MILLISECONDS_IN_SECONDS));  // Get Milliseconds

    return (double)epoch_secs + milliseconds;  
}


/**
 * @brief Sets RTC time based on CAN data formatted as follows:
 *        Byte 0: Seconds, Byte 1: Minutes, Byte 2: Hours
 * @param data: Pointer to the CAN data array
 * @return None
*/
static void set_rtc_time(uint8_t* data)
{
    /* Initialize Time Object */
    RTC_TimeTypeDef sTime = {0};

    /* Manually parsing the seconds minutes hours */
    sTime.Seconds = data[TIMETYPEDEF_SECONDS_IDX];
    sTime.Minutes = data[TIMETYPEDEF_MINUTES_IDX];
    sTime.Hours   = data[TIMETYPEDEF_HOURS_IDX];

    /* Set the RTC time with these settings */
    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
}


/**
 * @brief Sets RTC date based on CAN data formatted as follows:
 *       Byte 3: Date, Byte 4: Month, Byte 5: Year (from 2000)
 * @param data: Pointer to the CAN data array
 * @return None
*/
static void set_rtc_date(uint8_t* data)
{
    /* Initialize Date Object */
    RTC_DateTypeDef sDate = {0};

    /* Manually parsing the date, month, and year */
    sDate.Date  = data[TIMETYPEDEF_DAY_IDX];
    sDate.Month = data[TIMETYPEDEF_MONTH_IDX];
    sDate.Year  = data[TIMETYPEDEF_YEAR_IDX];

    /* Set the RTC Date with these settings */
    HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
}


/**
 * @brief Function to sync the memorator RTC to the TEL clock
 * @param rx_CAN_msg: Pointer to a CAN_msg_t
 * @return None
*/
static void sync_memorator_rtc(uint8_t* rtc_can_msg_data)
{
    rtc_set = true;                 

    set_rtc_time(rtc_can_msg_data);
    set_rtc_date(rtc_can_msg_data);
}


/**
 * @brief Function to check and sync the RTC with memorator message
 * @param rx_CAN_msg The received CAN message
 * @return void
*/
void RTC_check_and_sync_rtc(uint32_t can_id, uint8_t* rtc_can_msg_data) {
    /* Perform rtc syncing check if the message is 0x300 and if RTC is reset to 2000-01-01 */
    if (can_id == RTC_TIMESTAMP_MSG_ID && !rtc_set)
    {
        start_of_second = HAL_GetTick();
        sync_memorator_rtc(rtc_can_msg_data);
    }
}

/* USER CODE END 1 */
