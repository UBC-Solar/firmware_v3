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
#include "can.h"

/* USER CODE BEGIN 0 */
#include <stdbool.h>
#include <time.h>
#include "i2c.h"
#include "main.h"
#include "nmea_parse.h"

#define GPS_RCV_BUFFER_SIZE 512
//#define GPS_SYNC_TIMEOUT 60000 * 5 // 5 minutes
#define GPS_SYNC_TIMEOUT 1000 // 1 second

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

double get_current_timestamp()
{
  /* Initialize Time and Date objects */
  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef sDate;

  HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
  double epochTime = convertToEpochTime(&sTime, &sDate);

  /* Return the resulting epoch time */
  return epochTime;
}

/* Used to get current time stamp */
double convertToEpochTime(RTC_TimeTypeDef *sTime, RTC_DateTypeDef *sDate)
{
    /* Initialize tm struct - from time.h library */
    struct tm t;

    /* Populate the tm structure fields from the RTC time and date */
    t.tm_year = sDate->Year + 100;  // Year since 1900, so add 100 to the year from the RTC (which starts at 2000).
    t.tm_mon = sDate->Month - 1;    // tm_mon is 0-11, so subtract 1 from the RTC month (which starts at 1).
    t.tm_mday = sDate->Date;
    t.tm_hour = sTime->Hours;
    t.tm_min = sTime->Minutes;
    t.tm_sec = sTime->Seconds;
    t.tm_isdst = 0;                // Disable daylight saving time adjustments.

    /* Subtract 8 hours with roll-back feature */
//    t.tm_hour -= 8;
//    if (t.tm_hour < 0) {
//        t.tm_hour += 24;  // Adjust the hour to ensure it's not less than 0.
//        t.tm_mday--;      // Decrement the day to reflect the day change.
//
//        /* Adjust the month and year if needed when day rolls below 1 */
//        if (t.tm_mday < 1) {
//            t.tm_mon--;  // Decrement the month.
//            if (t.tm_mon < 0) { // If month rolls below January
//                t.tm_mon = 11; // Set month to December
//                t.tm_year--;   // Decrement the year
//            }
//            /* Set day to last day of the new month */
//            t.tm_mday = lastDayOfMonth(t.tm_mon, t.tm_year + 1900);
//        }
//    }

    /* Convert to epoch time - Function from time.h library */
    long int epoch_secs = (long int) mktime(&t);

    /* Convert to double and add milliseconds with GetTick() */
    return (double) epoch_secs + (double)(HAL_GetTick() % 1000) / 1000.0;
}

/* Function to return the last day of a month */
int lastDayOfMonth(int month, int year)
{
  /* Array to hold the number of days in each month */
  int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

  /* Check for leap year in February */
  if (month == 1) { // February case
    if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) {
      return 29;
    }
  }
  return daysInMonth[month];
}


/**
 * @brief Will check the current RTC time on the TEL board. If it is 2000-01-01
 *        then RTC is not synced (it is reset to default). Also sets tel_diagnostic.rtc_reset to appropriate val
 * @return boolean indicating if RTC reset (true) or not (false)
 */
bool checkAndSetRTCReset()
{
	RTC_DateTypeDef curr_date;
	RTC_TimeTypeDef curr_time;
	HAL_RTC_GetDate(&hrtc, &curr_date, RTC_FORMAT_BIN);

	/* Set rtc_reset true if current time is Jan 1, 2000 */
	if ((curr_date.Month == RTC_MONTH_JANUARY && curr_date.Date == 1 && curr_date.Year == 0) || HAL_GPIO_ReadPin(RTC_SYNC_GPIO_Port, RTC_SYNC_Pin) == GPIO_PIN_SET) {
		g_tel_diagnostics.rtc_reset = true;
		return true;
	}
	return false;
}
/**
 * @brief Sets RTC time based on CAN data formatted as follows:
 *        Byte 0: Seconds, Byte 1: Minutes, Byte 2: Hours
 * @param data: Pointer to the CAN data array
 * @return None
*/
void setRTCTime(uint8_t* data)
{
    /* Initialize Time Object */
    RTC_TimeTypeDef sTime = {0};

    /* Manually parsing the seconds minutes hours */
    sTime.Seconds = data[0];
    sTime.Minutes = data[1];
    sTime.Hours   = data[2];

    /* Set the RTC time with these settings */
    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
}


/**
 * @brief Sets RTC date based on CAN data formatted as follows:
 *       Byte 3: Date, Byte 4: Month, Byte 5: Year (from 2000)
 * @param data: Pointer to the CAN data array
 * @return None
*/
void setRTCDate(uint8_t* data)
{
    /* Initialize Date Object */
    RTC_DateTypeDef sDate = {0};

    /* Manually parsing the date, month, and year */
    sDate.Date  = data[3];
    sDate.Month = data[4];
    sDate.Year  = data[5];

    /* Set the RTC Date with these settings */
    HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
}


/**
 * @brief Function to sync the memorator RTC to the TEL clock
 * @param rx_CAN_msg: Pointer to a CAN_msg_t
 * @return None
*/
void sync_memorator_rtc(CAN_msg_t* rx_CAN_msg)
{
    /* Only perform syncing if RTC was reset */
    if (g_tel_diagnostics.rtc_reset) {
        /* Set the RTC time and date based on the CAN data */
        setRTCTime(rx_CAN_msg->data);
        setRTCDate(rx_CAN_msg->data);

        /* Set the flag to indicate that the RTC has been sync'd */
        g_tel_diagnostics.rtc_reset = false;
    }
}

/* USER CODE END 1 */
