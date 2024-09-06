/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    rtc.c
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
#include "main.h"
#include <stdbool.h>
#include <time.h>


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
 * @brief Function to convert the RTC time and date to epoch time in seconds and milliseconds
 * 
 * Precondition: The year in the date struct is relavtive to year 2000. Ex. year = 24 means 2024
 * 
 * @param sTime: Pointer to the RTC_TimeTypeDef structure
 * @param sDate: Pointer to the RTC_DateTypeDef structure
 * @param decimal_millis: The number of milliseconds since the last second as a decimal (ex. 500ms = 0.500)
 */
static double convert_to_epoch_time(RTC_TimeTypeDef *sTime, RTC_DateTypeDef *sDate, double decimal_millis)
{
    /* Initialize tm struct - from time.h library */
    struct tm t;

    /* Populate the tm structure fields from the RTC time and date */
    t.tm_year = sDate->Year + TEL_TO_MEMORATOR_RTC_YEAR_OFFSET; 
    t.tm_mon = sDate->Month - 1;    // tm_mon is 0-11, so subtract 1 from the RTC month (which starts at 1).
    t.tm_mday = sDate->Date;
    t.tm_hour = sTime->Hours;
    t.tm_min = sTime->Minutes;
    t.tm_sec = sTime->Seconds;
    t.tm_isdst = 0;                // Disable daylight saving time adjustments.

    long int epoch_secs = (long int) mktime(&t);    // mktime() from time.h converts to epoch for us

    return (double)epoch_secs + decimal_millis;     // return in seconds so milliseconds added as a decimal
}


/**
 * @brief Function to get the current timestamp in epoch time in seconds with milliseconds as a decimal
 */
double RTC_get_current_timestamp()
{
  /* Initialize Time and Date objects */
  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef sDate;

  HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
  // double decimal_millis = (HAL_GetTick() - start_of_second) / 1000.0;
  double decimal_millis = HAL_GetTick() / MILLIS_IN_SECOND;

  return convert_to_epoch_time(&sTime, &sDate, decimal_millis);
}


/**
 * @brief Function to check if the RTC has been reset to 2000-01-01 (STM32 default)
 */
static bool is_rtc_reset(RTC_DateTypeDef* date)
{
    return date->Year == 0 && date->Month == RTC_MONTH_JANUARY && date->Date == 1;
}


/**
 * @brief Will check the current RTC time on the TEL board. If it is 2000-01-01
 *        then RTC is not synced (it is reset to default). Also sets tel_diagnostic.rtc_reset to appropriate val
 * @return boolean indicating if RTC reset (true) or not (false)
 */
bool RTC_check_and_set_reset_flag()
{
	RTC_DateTypeDef curr_date;
	HAL_RTC_GetDate(&hrtc, &curr_date, RTC_FORMAT_BIN);

  bool rtc_reset = is_rtc_reset(&curr_date);    

  g_tel_diagnostics.rtc_reset = rtc_reset;

  return rtc_reset;     // use local copy for safety. although this global field is currently only written to here.
}


/**
 * @brief Sets RTC time based on CAN data formatted as follows:
 *        Byte 0: Seconds, Byte 1: Minutes, Byte 2: Hours
 * @param data: Pointer to the CAN data array of RTC Timestamp message
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
static void sync_memorator_rtc(uint8_t* data)
{
    /* Only perform syncing if RTC was reset */
    if (g_tel_diagnostics.rtc_reset) 
    {
        /* Set the RTC time and date based on the CAN data */
        set_rtc_time(data);
        set_rtc_date(data);

        /* Set the flag to indicate that the RTC has been sync'd */
        g_tel_diagnostics.rtc_reset = false;
    }
}


/**
 * @brief Function to check and sync the RTC with memorator message
 * @param rx_CAN_msg The received CAN message
 * @return void
*/
void RTC_check_and_sync_rtc(uint32_t can_id, uint8_t *data) 
{
    /* Perform rtc syncing check if the message is 0x300 and if RTC is reset to 2000-01-01 */
    if (can_id == RTC_TIMESTAMP_MSG_ID && RTC_check_and_set_reset_flag())
    {
        sync_memorator_rtc(data);
    }
}

/* USER CODE END 1 */
