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
#include "nmea_parse.h"
#include "i2c.h"
#include <time.h>

#define GPS_RCV_BUFFER_SIZE 512

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


void Sync_RTC_With_GPS()
{
  /* Initialize a receive buffer */
  uint8_t receive_buffer[GPS_RCV_BUFFER_SIZE];

  /* Buffers for the GPSTime and GPSDate */
  char GPSTime[256];
  char GPSDate[256];

  /* Flag to track if the sync is complete */
  uint8_t RTC_Sync_Flag = 0;

  while(RTC_Sync_Flag == 0) {
    /* Read in an NMEA message into the buffer */
    readNMEA(&receive_buffer);

    /*
     * Get the date and time and Sync the RTC
     * The function will set RTC_Sync_Flag = 1 when the RTC is sync'd
     */
    getGPSDateTime(receive_buffer, GPSTime, GPSDate, &RTC_Sync_Flag);
  }

  /* Can turn on the TEL board LED here to indicate that the RTC is SYNC'd  */
}


void getGPSDateTime(uint8_t *buffer, char *GPSTime, char *GPSDate, uint8_t *RTC_Sync_Flag)
{
  GPS myData;
  nmea_parse(&myData, buffer);

  /*
   * lastMeasure is a null-terminated string and has the format hhmmss.sss
   * Make sure there's a valid fix and that there is an RMC message
   */
  if(myData.fix == 1 && myData.RMC_Flag == 1) {

    /* Copy the GPS time to GPSTime */
    strncpy(GPSTime, myData.lastMeasure, 10);
    GPSTime[10] = '\0'; // Ensure null termination

    /* Copy the GPS date to GPSDate */
    strncpy(GPSDate, myData.date, 6);
    GPSDate[6] = '\0'; // Ensure null termination

    /* Initialize Time and Date Objects */
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    /* Manually parsing the hours, minutes, and seconds */
    sTime.Hours   = (GPSTime[0] - '0') * 10 + (GPSTime[1] - '0');
    sTime.Minutes = (GPSTime[2] - '0') * 10 + (GPSTime[3] - '0');
    sTime.Seconds = (GPSTime[4] - '0') * 10 + (GPSTime[5] - '0');

    /* Set the RTC time with these settings */
    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

    /* Manually parsing the date, month, and year */
    sDate.Date  = (GPSDate[0] - '0') * 10 + (GPSDate[1] - '0');
    sDate.Month = (GPSDate[2] - '0') * 10 + (GPSDate[3] - '0');
    sDate.Year  = (GPSDate[4] - '0') * 10 + (GPSDate[5] - '0');

    /* Set the RTC Date with these settings */
    HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    /* Set the flag to 1 indicating that the RTC has been sync'd */
    *RTC_Sync_Flag = 1;
  }
}

time_t get_current_timestamp()
{
  /* Initialize Time and Date objects */
  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef sDate;

  HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

  /* Convert to Epoch Time */
  time_t epochTime = convertToEpochTime(&sTime, &sDate);

  /* Return the resulting epoch time */
  return epochTime;
}

/* Used to get current time stamp */
time_t convertToEpochTime(RTC_TimeTypeDef *sTime, RTC_DateTypeDef *sDate)
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
    t.tm_isdst = -1;                // Disable daylight saving time adjustments.

    /* Subtract 8 hours with roll-back feature */
    t.tm_hour -= 8;
    if (t.tm_hour < 0) {
        t.tm_hour += 24;  // Adjust the hour to ensure it's not less than 0.
        t.tm_mday--;      // Decrement the day to reflect the day change.

        /* Adjust the month and year if needed when day rolls below 1 */
        if (t.tm_mday < 1) {
            t.tm_mon--;  // Decrement the month.
            if (t.tm_mon < 0) { // If month rolls below January
                t.tm_mon = 11; // Set month to December
                t.tm_year--;   // Decrement the year
            }
            /* Set day to last day of the new month */
            t.tm_mday = lastDayOfMonth(t.tm_mon, t.tm_year + 1900);
        }
    }

    /* Convert to epoch time - Function from time.h library */
    return mktime(&t);
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

/* USER CODE END 1 */
