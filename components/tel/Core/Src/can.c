/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.c
  * @brief   This file provides code for the configuration
  *          of the CAN instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "can.h"

/* USER CODE BEGIN 0 */

/* PRIVATE INCLUDES */
#include <stdint.h>
#include <stdbool.h>
#include "bitops.h"
#include "radio.h"
#include "CAN_comms.h"
#include "rtc.h"
#include "nmea_parse.h"

#define GPS_CAN_MESSAGE_LENGTH                         8
#define GPS_CAN_MESSAGE_INT_LENGTH                     4

#define GPS_DATA_SAT_COUNT_VIEW_FIX_SNR_CAN_MESSAGE_ID 0x755
#define GPS_DATA_LON_LAT_CAN_MESSAGE_ID                0x756
#define GPS_DATA_HDOP_VDOP_CAN_MESSAGE_ID              0x757
#define GPS_DATA_LON_SIDE_DATE_CAN_MESSAGE_ID          0x758 // UPDATE TO 758
#define GPS_DATA_PDOP_SPEEDKMH_CAN_MESSAGE_ID          0x760 // 759
#define GPS_DATA_ALT_GEOD_CAN_MESSAGE_ID               0x761 // 760
#define GPS_DATA_TRUE_MAG_HEADING_CAN_MESSAGE_ID       0x762 // 761

CAN_TxHeaderTypeDef gps_sat_count_view_fix_snr_rmc = {
  .StdId = GPS_DATA_SAT_COUNT_VIEW_FIX_SNR_CAN_MESSAGE_ID,
  .IDE = CAN_ID_STD,
  .RTR = CAN_RTR_DATA,
  .DLC = GPS_CAN_MESSAGE_INT_LENGTH
};

CAN_TxHeaderTypeDef gps_lon_lat = {
  .StdId = GPS_DATA_LON_LAT_CAN_MESSAGE_ID,
  .ExtId = 0x0000,
  .IDE = CAN_ID_STD,
  .RTR = CAN_RTR_DATA,
  .DLC = GPS_CAN_MESSAGE_LENGTH
};

CAN_TxHeaderTypeDef gps_hdop_vdop = {
  .StdId = GPS_DATA_HDOP_VDOP_CAN_MESSAGE_ID,
  .ExtId = 0x0000,
  .IDE = CAN_ID_STD,
  .RTR = CAN_RTR_DATA,
  .DLC = GPS_CAN_MESSAGE_LENGTH
};

CAN_TxHeaderTypeDef gps_alt_geod = {
  .StdId = GPS_DATA_ALT_GEOD_CAN_MESSAGE_ID,
  .ExtId = 0x0000,
  .IDE = CAN_ID_STD,
  .RTR = CAN_RTR_DATA,
  .DLC = GPS_CAN_MESSAGE_LENGTH
};

// CAN_TxHeaderTypeDef gps_lat_side_utc_time = {
//   .StdId = GPS_DATA_LAT_SIDE_UTC_TIME_CAN_MESSAGE_ID,
//   .ExtId = 0x0000,
//   .IDE = CAN_ID_STD,
//   .RTR = CAN_RTR_DATA,
//   .DLC = GPS_CAN_MESSAGE_LENGTH
// };

CAN_TxHeaderTypeDef gps_lon_side_date = {
  .StdId = GPS_DATA_LON_SIDE_DATE_CAN_MESSAGE_ID,
  .ExtId = 0x0000,
  .IDE = CAN_ID_STD,
  .RTR = CAN_RTR_DATA,
  .DLC = GPS_CAN_MESSAGE_LENGTH
};

CAN_TxHeaderTypeDef gps_pdop_speedkmh = {
  .StdId = GPS_DATA_PDOP_SPEEDKMH_CAN_MESSAGE_ID,
  .ExtId = 0x0000,
  .IDE = CAN_ID_STD,
  .RTR = CAN_RTR_DATA,
  .DLC = GPS_CAN_MESSAGE_LENGTH
};

CAN_TxHeaderTypeDef gps_true_mag_heading = {
  .StdId = GPS_DATA_TRUE_MAG_HEADING_CAN_MESSAGE_ID,
  .ExtId = 0x0000,
  .IDE = CAN_ID_STD,
  .RTR = CAN_RTR_DATA,
  .DLC = GPS_CAN_MESSAGE_LENGTH
};
/* USER CODE END 0 */

CAN_HandleTypeDef hcan;

/* CAN init function */
void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN1;
  hcan.Init.Prescaler = 8;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_4TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_4TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = DISABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = DISABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN_Init 2 */

  /* USER CODE END CAN_Init 2 */

}

void HAL_CAN_MspInit(CAN_HandleTypeDef* canHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspInit 0 */

  /* USER CODE END CAN1_MspInit 0 */
    /* CAN1 clock enable */
    __HAL_RCC_CAN1_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**CAN GPIO Configuration
    PB8     ------> CAN_RX
    PB9     ------> CAN_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    __HAL_AFIO_REMAP_CAN1_2();

    /* CAN1 interrupt Init */
    HAL_NVIC_SetPriority(USB_HP_CAN1_TX_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USB_HP_CAN1_TX_IRQn);
    HAL_NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
  /* USER CODE BEGIN CAN1_MspInit 1 */

  /* USER CODE END CAN1_MspInit 1 */
  }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* canHandle)
{

  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspDeInit 0 */

  /* USER CODE END CAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CAN1_CLK_DISABLE();

    /**CAN GPIO Configuration
    PB8     ------> CAN_RX
    PB9     ------> CAN_TX
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8|GPIO_PIN_9);

    /* CAN1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USB_HP_CAN1_TX_IRQn);
    HAL_NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn);
  /* USER CODE BEGIN CAN1_MspDeInit 1 */

  /* USER CODE END CAN1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/**
 * @brief Callback from CAN comms to send a message over UART
 * 
 * @param CAN_comms_Rx_msg The CAN message received
 */
void CAN_comms_Rx_callback(CAN_comms_Rx_msg_t* CAN_comms_Rx_msg)
{
    HAL_GPIO_TogglePin(USER_LED_GPIO_Port, USER_LED_Pin);	    // Visual Confirmation of CAN working

    RTC_check_and_sync_rtc(CAN_comms_Rx_msg->header.StdId, CAN_comms_Rx_msg->data);     // Sync timestamps

    RADIO_send_msg_uart(&(CAN_comms_Rx_msg->header), CAN_comms_Rx_msg->data);
}


/**
 * @brief Initializes the CAN filter and CAN Rx callback function as CAN_comms_Rx_callback().
 * 
 * Note: This uses the CAN_comms abstraction layer which will initialize two freeRTOS tasks. As a result it is recommend to 
 * Call this function inside the MX_FREERTOS_Init() function in freertos.c
 */
void CAN_tasks_init()
{
    CAN_comms_config_t CAN_comms_config_tel = {0}; 

    CAN_FilterTypeDef CAN_filter = {0};
    CAN_filter_init(&CAN_filter);

    CAN_comms_config_tel.hcan = &hcan;
    CAN_comms_config_tel.CAN_Filter = CAN_filter;
    CAN_comms_config_tel.CAN_comms_Rx_callback = CAN_comms_Rx_callback;   

    CAN_comms_init(&CAN_comms_config_tel);
}


/**
 * @brief Allows all messages to be received
 */
void CAN_filter_init(CAN_FilterTypeDef* can_filter)
{
    /* TODO: Review Filter Implementation */
    // Use mask and list mode to filter IDs from the CAN ID BOM

    can_filter->FilterIdHigh = 0x0000;
    can_filter->FilterMaskIdHigh = 0x0000;

    can_filter->FilterIdLow = 0x0000;
    can_filter->FilterMaskIdLow = 0x0000;

    can_filter->FilterFIFOAssignment = CAN_FILTER_FIFO0;
    can_filter->FilterBank = (uint32_t) 0;
    can_filter->FilterMode = CAN_FILTERMODE_IDMASK;
    can_filter->FilterScale = CAN_FILTERSCALE_16BIT;
    can_filter->FilterActivation = CAN_FILTER_ENABLE;
}


/**
 * @brief Callback function for when a CAN error occurs
 * 
 * Currently only toggles the user LED upon a RX FIFO overrun
 */
void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan)
{
    // TODO: Add diagnostics. Dont toggle LED if you already do in Rx
    if (hcan->ErrorCode & HAL_CAN_ERROR_RX_FOV1)
    {
        HAL_GPIO_TogglePin(USER_LED_GPIO_Port, USER_LED_Pin);   // TODO: add diagnostics
    }
}

union {
      float value;
      uint8_t bytes[4];
    } float_bytes;

void CAN_tx_lon_lat_msg(float latitude, float longitude) {

    float_bytes.value = latitude;

    CAN_comms_Tx_msg_t CAN_comms_Tx_msg = {
        .header = gps_lon_lat
    };

    for (int i = 0; i < 4; i++) 
    {
      CAN_comms_Tx_msg.data[i] = float_bytes.bytes[i];
    }

    float_bytes.value = longitude;

    for (int i = 0; i < 4; i++) 
    {
      CAN_comms_Tx_msg.data[i + 4] = float_bytes.bytes[i];
    }

  CAN_comms_Add_Tx_message(&CAN_comms_Tx_msg);
}

void CAN_tx_alt_geod_msg(float altitude, float geodHeight) {

    float_bytes.value = altitude;

    CAN_comms_Tx_msg_t CAN_comms_Tx_msg = {
        .header = gps_alt_geod
    };

    for (int i = 0; i < 4; i++) 
    {
      CAN_comms_Tx_msg.data[i] = float_bytes.bytes[i];
    }

    float_bytes.value = geodHeight;

    for (int i = 0; i < 4; i++) 
    {
      CAN_comms_Tx_msg.data[i + 4] = float_bytes.bytes[i];
    }

  CAN_comms_Add_Tx_message(&CAN_comms_Tx_msg);
}

void CAN_tx_hdop_vdop_msg(float hdop, float vdop) {

    float_bytes.value = hdop;

    CAN_comms_Tx_msg_t CAN_comms_Tx_msg = {
        .header = gps_hdop_vdop
    };

    for (int i = 0; i < 4; i++) 
    {
      CAN_comms_Tx_msg.data[i] = float_bytes.bytes[i];
    }

    float_bytes.value = vdop;

    for (int i = 0; i < 4; i++) 
    {
      CAN_comms_Tx_msg.data[i + 4] = float_bytes.bytes[i];
    }

  CAN_comms_Add_Tx_message(&CAN_comms_Tx_msg);
}

void CAN_tx_pdop_speedKmh_msg(float pdop, float speedKmh) {

    float_bytes.value = pdop;

    CAN_comms_Tx_msg_t CAN_comms_Tx_msg = {
        .header = gps_pdop_speedkmh
    };

    for (int i = 0; i < 4; i++) 
    {
      CAN_comms_Tx_msg.data[i] = float_bytes.bytes[i];
    }

    float_bytes.value = speedKmh;

    for (int i = 0; i < 4; i++) 
    {
      CAN_comms_Tx_msg.data[i + 4] = float_bytes.bytes[i];
    }

  CAN_comms_Add_Tx_message(&CAN_comms_Tx_msg);
}

void CAN_tx_true_magnetic_heading_msg(float trueHeading, float magneticHeading) {

    float_bytes.value = trueHeading;

    CAN_comms_Tx_msg_t CAN_comms_Tx_msg = {
        .header = gps_true_mag_heading
    };  

    for (int i = 0; i < 4; i++) 
    {
      CAN_comms_Tx_msg.data[i] = float_bytes.bytes[i];
    }

    float_bytes.value = magneticHeading;

    for (int i = 0; i < 4; i++) 
    {
      CAN_comms_Tx_msg.data[i + 4] = float_bytes.bytes[i];
    }

  CAN_comms_Add_Tx_message(&CAN_comms_Tx_msg);
}

void CAN_tx_sat_count_view_fix_snr_msg(int satelliteCount, int satInView, int fix, int snr) {

    CAN_comms_Tx_msg_t CAN_comms_Tx_msg = {
      .data[0] = (uint8_t) satelliteCount,
      .data[1] = (uint8_t) satInView,
      .data[2] = (uint8_t) fix,
      .data[3] = (uint8_t) snr,

      .header = gps_sat_count_view_fix_snr_rmc
    };
  CAN_comms_Add_Tx_message(&CAN_comms_Tx_msg);
}

// void CAN_tx_lat_side_utc_time_msg(char latSide, char utcTime[7]) {
//     CAN_comms_Tx_msg_t CAN_comms_Tx_msg = {
//         .data[7] = (uint8_t) latSide,
//         .header = gps_lat_side_utc_time
//     };  

//   // ... copy from below

//   CAN_comms_Add_Tx_message(&CAN_comms_Tx_msg);
// }

void CAN_tx_lon_side_date_msg(char lonSide, char latSide, char date[7], char utcTime[7]) {
    CAN_comms_Tx_msg_t CAN_comms_Tx_msg = {
        .data[6] = (uint8_t) latSide,
        .data[7] = (uint8_t) lonSide,
        .header = gps_lon_side_date
    };

    int date_int[6];

    char date1[3], date2[3], date3[3];
    char utcTime1[3], utcTime2[3], utcTime3[3];

    date1[0] = date[0];
    date1[1] = date[1];
    date1[2] = '\0';

    date2[0] = date[2];
    date2[1] = date[3];
    date2[2] = '\0';
    
    date3[0] = date[4];
    date3[1] = date[5];
    date3[2] = '\0';

    date_int[0] = atoi(date1);
    date_int[1] = atoi(date2);
    date_int[2] = atoi(date3);

    utcTime1[0] = utcTime[0];
    utcTime1[1] = utcTime[1];
    utcTime1[2] = '\0';

    utcTime2[0] = utcTime[2];
    utcTime2[1] = utcTime[3];
    utcTime2[2] = '\0';

    utcTime3[0] = utcTime[4];
    utcTime3[1] = utcTime[5];
    utcTime3[2] = '\0';

    date_int[3] = atoi(utcTime1);
    date_int[4] = atoi(utcTime2);
    date_int[5] = atoi(utcTime3);

    for (int i = 0; i < 6; i++)
    {
      CAN_comms_Tx_msg.data[i] = date_int[i];
    }

  CAN_comms_Add_Tx_message(&CAN_comms_Tx_msg);
}

void CAN_tx_gps_data_msg(GPS* gps_data) {
  CAN_tx_lon_lat_msg(gps_data->longitude, gps_data->latitude);
  CAN_tx_alt_geod_msg(gps_data->altitude, gps_data->geodHeight);
  CAN_tx_hdop_vdop_msg(gps_data->hdop, gps_data->vdop);
  CAN_tx_pdop_speedKmh_msg(gps_data->pdop, gps_data->speedKmh);
  CAN_tx_true_magnetic_heading_msg(gps_data->trueHeading, gps_data->magneticHeading);
  CAN_tx_sat_count_view_fix_snr_msg(gps_data->satelliteCount, gps_data->satInView, gps_data->fix, gps_data->snr);
  // CAN_tx_lat_side_utc_time_msg(gps_data->latSide, gps_data->utcTime);
  CAN_tx_lon_side_date_msg(gps_data->lonSide, gps_data->latSide, gps_data->date, gps_data->utcTime);
}
/* USER CODE END 1 */