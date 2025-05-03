/******************************************************************************
* @file    gps.c
* @brief   Sets g_gps_read_okay to true or false based on I2C reading
******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "gps.h"
#include "nmea_parse.h"
#include "can.h"
#include "diagnostic.h"
#include "CAN_comms.h"
#include "radio.h"
#include "bitops.h"

#define GPS_DEVICE_ADDRESS ((0x42)<<1)

bool g_gps_read_okay = false;
uint8_t g_gps_data[GPS_MESSAGE_LEN];
char gps_parse_data[GPS_MESSAGE_LEN];


/** CAN HEADERS */
#define GPS_CAN_MESSAGE_LENGTH                         8
#define GPS_CAN_MESSAGE_INT_LENGTH                     4

/**
 * @brief CAN message header for GPS messages
 */
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



 /**
 * @brief Callback function triggered when an I2C master receive operation completes.
 * @param hi2c Pointer to the I2C handle structure that triggered the callback
 */
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    /* Prevent unused argument(s) compilation warning */
    if(hi2c->Instance == I2C1)
    {
        g_gps_read_okay = true;
    }
}

/**
 * @brief Continually tries to get a fix and sets if the GPS messages are read or not
 * @param receive_buffer The buffer to store the received data
 */
void read_i2c_gps_module(uint8_t* receive_buffer)
{
	g_gps_read_okay = false;
    HAL_StatusTypeDef status = HAL_I2C_Master_Receive_IT(&hi2c1, GPS_DEVICE_ADDRESS, receive_buffer, GPS_MESSAGE_LEN);
    if(status == HAL_OK)
    {
        g_tel_diagnostic_flags.bits.gps_read_fail = false;
    }
    else
    {
        g_tel_diagnostic_flags.bits.gps_read_fail = true;
    }
}
/**
 * @brief Reads the GPS data and confirms if it is read to be parsed into gps_data 
 */
void gps_task()
{
    if (g_gps_read_okay)
    {
        GPS gps_data = {0};
        
        nmea_parse(&gps_data, g_gps_data);
        
        CAN_tx_gps_data_msg(&gps_data);
        
        HAL_GPIO_TogglePin(USER_LED_GPIO_Port, USER_LED_Pin);
    }
    
    memset(g_gps_data, 0, GPS_MESSAGE_LEN);
    read_i2c_gps_module(g_gps_data);
}

/** CAN SENDING LOGIC */
/**
 * @brief CAN message for latitude and longitude GPS fields
 */
void CAN_tx_lon_lat_msg(float latitude, float longitude) {
    FloatToBytes float_bytes;
    float_bytes.f = latitude;

    CAN_comms_Tx_msg_t CAN_comms_Tx_msg = {
        .header = gps_lon_lat
    };

    for (int i = 0; i < 4; i++) 
    {
      CAN_comms_Tx_msg.data[i] = float_bytes.bytes[i];
    }

    float_bytes.f = longitude;

    for (int i = 0; i < 4; i++) 
    {
      CAN_comms_Tx_msg.data[i + 4] = float_bytes.bytes[i];
    }

  CAN_comms_Add_Tx_message(&CAN_comms_Tx_msg);
  RADIO_filter_and_queue_msg_tx(&CAN_comms_Tx_msg);  
}


/**
 * @brief CAN message for altitude and geodHeight GPS fields
 */
void CAN_tx_alt_geod_msg(float altitude, float geodHeight) {
    FloatToBytes float_bytes;
    float_bytes.f = altitude;

    CAN_comms_Tx_msg_t CAN_comms_Tx_msg = {
        .header = gps_alt_geod
    };

    for (int i = 0; i < 4; i++) 
    {
      CAN_comms_Tx_msg.data[i] = float_bytes.bytes[i];
    }

    float_bytes.f = geodHeight;

    for (int i = 0; i < 4; i++) 
    {
      CAN_comms_Tx_msg.data[i + 4] = float_bytes.bytes[i];
    }

  CAN_comms_Add_Tx_message(&CAN_comms_Tx_msg);
  RADIO_filter_and_queue_msg_tx(&CAN_comms_Tx_msg);  
}


/**
 * @brief CAN message for hdop and vdop GPS fields
 */
void CAN_tx_hdop_vdop_msg(float hdop, float vdop) {
    FloatToBytes float_bytes;
    float_bytes.f = hdop;

    CAN_comms_Tx_msg_t CAN_comms_Tx_msg = {
        .header = gps_hdop_vdop
    };

    for (int i = 0; i < 4; i++) 
    {
      CAN_comms_Tx_msg.data[i] = float_bytes.bytes[i];
    }

    float_bytes.f = vdop;

    for (int i = 0; i < 4; i++) 
    {
      CAN_comms_Tx_msg.data[i + 4] = float_bytes.bytes[i];
    }

  CAN_comms_Add_Tx_message(&CAN_comms_Tx_msg);
  RADIO_filter_and_queue_msg_tx(&CAN_comms_Tx_msg);  
}


/**
 * @brief CAN message for pdop and speedKmh GPS fields
 */
void CAN_tx_pdop_speedKmh_msg(float pdop, float speedKmh) {
    FloatToBytes float_bytes;
    float_bytes.f = pdop;

    CAN_comms_Tx_msg_t CAN_comms_Tx_msg = {
        .header = gps_pdop_speedkmh
    };

    for (int i = 0; i < 4; i++) 
    {
      CAN_comms_Tx_msg.data[i] = float_bytes.bytes[i];
    }

    float_bytes.f = speedKmh;

    for (int i = 0; i < 4; i++) 
    {
      CAN_comms_Tx_msg.data[i + 4] = float_bytes.bytes[i];
    }

  CAN_comms_Add_Tx_message(&CAN_comms_Tx_msg);
  RADIO_filter_and_queue_msg_tx(&CAN_comms_Tx_msg);  
}


/**
 * @brief CAN message for trueHeading and magneticHeading GPS fields
 */
void CAN_tx_true_magnetic_heading_msg(float trueHeading, float magneticHeading) {
    FloatToBytes float_bytes;
    float_bytes.f = trueHeading;

    CAN_comms_Tx_msg_t CAN_comms_Tx_msg = {
        .header = gps_true_mag_heading
    };  

    for (int i = 0; i < 4; i++) 
    {
      CAN_comms_Tx_msg.data[i] = float_bytes.bytes[i];
    }

    float_bytes.f = magneticHeading;

    for (int i = 0; i < 4; i++) 
    {
      CAN_comms_Tx_msg.data[i + 4] = float_bytes.bytes[i];
    }

  CAN_comms_Add_Tx_message(&CAN_comms_Tx_msg);
  RADIO_filter_and_queue_msg_tx(&CAN_comms_Tx_msg);
}

/**
 * @brief CAN message for sateilliteCount, satInView, fix and snr GPS fields
 * 
 * Packages the following integer fields into a 4 byte array
 */
void CAN_tx_sat_count_view_fix_snr_msg(int satelliteCount, int satInView, int fix, int snr) {

    CAN_comms_Tx_msg_t CAN_comms_Tx_msg = {
      .data[0] = (uint8_t) satelliteCount,
      .data[1] = (uint8_t) satInView,
      .data[2] = (uint8_t) fix,
      .data[3] = (uint8_t) snr,

      .header = gps_sat_count_view_fix_snr_rmc
    };
  CAN_comms_Add_Tx_message(&CAN_comms_Tx_msg);
  RADIO_filter_and_queue_msg_tx(&CAN_comms_Tx_msg);
}


/**
 * @brief CAN message for lonSide, latSide, date and time GPS fields
 *
 * Packs the following fields into an 8 byte array and seperates the
 * date and time fields into 3 different integer arrays each from a single
 * character array
 */
void CAN_tx_lon_side_date_msg(char lonSide, char latSide, char date[7], char utcTime[7]) {
    CAN_comms_Tx_msg_t CAN_comms_Tx_msg = {
        .data[6] = (uint8_t) latSide,
        .data[7] = (uint8_t) lonSide,
        .header = gps_lon_side_date
    };

    int date_int[6];

    char date1[3], date2[3], date3[3];
    char utcTime1[3], utcTime2[3], utcTime3[3];

    // Hardcoding to split date and utcTime into 3 different arrays
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
  RADIO_filter_and_queue_msg_tx(&CAN_comms_Tx_msg);
}

/**
 * @brief Calls all CAN messages for every GPS field
 * @param gps_data Pointer to GPS struct of all fields
 */
void CAN_tx_gps_data_msg(GPS* gps_data) {
    osDelay(GPS_TASK_DELAY_SINGLE);
    CAN_tx_sat_count_view_fix_snr_msg(gps_data->satelliteCount, gps_data->satInView, gps_data->fix, gps_data->snr);
    osDelay(GPS_TASK_DELAY_SINGLE);
    CAN_tx_lon_lat_msg(gps_data->longitude, gps_data->latitude);
    osDelay(GPS_TASK_DELAY_SINGLE);
    CAN_tx_lon_side_date_msg(gps_data->lonSide, gps_data->latSide, gps_data->date, gps_data->utcTime);
    osDelay(GPS_TASK_DELAY_SINGLE);

    /** TO BE USED WHEN NECESSARY */
    // CAN_tx_hdop_vdop_msg(gps_data->hdop, gps_data->vdop);
    // osDelay(3);
    // CAN_tx_pdop_speedKmh_msg(gps_data->pdop, gps_data->speedKmh);
    // osDelay(3);
    // CAN_tx_alt_geod_msg(gps_data->altitude, gps_data->geodHeight);
    // osDelay(3);
    // CAN_tx_true_magnetic_heading_msg(gps_data->trueHeading, gps_data->magneticHeading);
    // osDelay(3);
}
