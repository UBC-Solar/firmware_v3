
/**
 *  @file gps.c
 *  @brief Defines functions for parsing and sending GPS messages as CAN messages
 *
 *  @date 2023/03/18
 *  @author Diego Armstrong
 */

#include "gps.h"


/**
 * @brief Reads in NMEA data from the GPS module and stores it in the GPS struct
 * @param gps_data: GPS struct to store the parsed data
 * @param receive_buffer: buffer to store the NMEA data
 * @return void
*/
static void read_in_NMEA_data(GPS* gps_data, uint8_t* receive_buffer)
{
    /* Read in an NMEA message into the buffer */
    if(HAL_I2C_IsDeviceReady(&hi2c1, GPS_DEVICE_ADDRESS, GPS_TRIALS, HAL_MAX_DELAY) == HAL_OK) {
	    HAL_I2C_Master_Receive(&hi2c1, GPS_DEVICE_ADDRESS, receive_buffer, sizeof(receive_buffer), HAL_MAX_DELAY);
    }

    /* Parse the buffer data --> gets stored in gps_data; */
    nmea_parse(&gps_data, &receive_buffer);
}


/**
 * @brief Creates a CAN message from GPS data
 * @param gps_data: GPS struct containing the GPS data
 * @return void
*/
static void create_and_send_CAN_from_GPS(GPS* gps_data) {    
    /* Get current epoch Time Stamp */
    union Utils_DoubleBytes_t current_timestamp;
    current_timestamp.double_value = get_current_timestamp();

    CAN_Radio_msg_t latitude_msg, longitude_msg, altitude_hdop_msg, side_and_count_msg;
    union Utils_DoubleBytes_t latitude_bytes, longitude_bytes;
    union Utils_FloatBytes_t altitude_bytes, hdop_bytes;

    /* Assign headers */
    latitude_msg.header = GPS_latitude_header;
    longitude_msg.header = GPS_longitude_header;
    altitude_hdop_msg.header = GPS_altitude_hdop_header;
    side_and_count_msg.header = GPS_side_count_header;

    /* Assign timestamps */
    latitude_msg.timestamp = current_timestamp;
    longitude_msg.timestamp = current_timestamp;
    altitude_hdop_msg.timestamp = current_timestamp;
    side_and_count_msg.timestamp = current_timestamp;

    /* Assign data as double/float so it can be read as uint64/uint8x4 */
    latitude_bytes.double_value = gps_data->latitude;
    longitude_bytes.double_value = gps_data->longitude;
    altitude_bytes.float_value = gps_data->altitude;
    hdop_bytes.float_value = gps_data->hdop;

    for  (uint8_t i = 0; i < CAN_DATA_LENGTH; i++) {                // Want to make GPS data into CAN message
        latitude_msg.data[DOUBLE_LAST_BYTE_IDX - i] = UTILS_GET_BYTE_AT_INDEX(i, latitude_bytes.double_as_int);
        longitude_msg.data[DOUBLE_LAST_BYTE_IDX - i] = UTILS_GET_BYTE_AT_INDEX(i, longitude_bytes.double_as_int);
        altitude_hdop_msg.data[FLOAT_LAST_BYTE_IDX - i] = altitude_bytes.bytes[i];                
        altitude_hdop_msg.data[DOUBLE_LAST_BYTE_IDX - i] = hdop_bytes.bytes[i];
    }

    /* Satellite Count Cast */
    uint32_t sat_count = (uint32_t) gps_data->satelliteCount;
    side_and_count_msg.data[LATSIDE_MSG_IDX] = gps_data->latSide;
    side_and_count_msg.data[LONSIDE_MSG_IDX] = gps_data->lonSide;
    for  (uint8_t i = 0; i < NUM_BYTES_FLOAT; i++) {
        side_and_count_msg.data[GPS_SIDE_AND_COUNT_DATA_END_INDEX - i] = ((sat_count >> (BITS_IN_BYTE * i)) && MASK_8_BITS);
    }
    side_and_count_msg.data[INDEX_6] = UNINITIALIZED;
    side_and_count_msg.data[INDEX_7] = UNINITIALIZED;

    /* Transmit GPS messages as CAN */
    send_GPS_as_CAN(&latitude_msg, &longitude_msg, &altitude_hdop_msg, &side_and_count_msg);
}


/**
 * @brief Sends GPS messages as CAN messages
 * @param latitude_msg: CAN message for latitude
 * @param longitude_msg: CAN message for longitude
 * @param altitude_hdop_msg: CAN message for altitude and hdop
 * @param side_and_count_msg: CAN message for side and count
 * @return void
*/
void send_GPS_as_CAN(CAN_Radio_msg_t* latitude_msg, CAN_Radio_msg_t* longitude_msg, CAN_Radio_msg_t* altitude_hdop_msg, CAN_Radio_msg_t* side_and_count_msg) {
    /* Transmit GPS messages as CAN */
    CAN_radio_and_bus_transmit(&hcan, latitude_msg, &can_mailbox);
    osDelay(GPS_SINGLE_MSG_DELAY);
    
    CAN_radio_and_bus_transmit(&hcan, altitude_hdop_msg, &can_mailbox);
    osDelay(GPS_SINGLE_MSG_DELAY);

    CAN_radio_and_bus_transmit(&hcan, side_and_count_msg, &can_mailbox);
    osDelay(GPS_NEXT_MSG_DELAY); // 4000 so we have 2 + 2 + 2 + 4 = 10 seconds total
}


/**
 * @brief Checks GPS fix and sends GPS data as CAN message if there is a fix
 * @param gps_data: GPS struct containing the GPS data
 * @return void
*/
static void check_GPS_fix_for_send(GPS* gps_data) {
    /* Only want to package GPS message if there is a fix. Otherwise no need */
    if (gps_data->fix == GOT_FIX)
    {
    	g_tel_diagnostics.gps_fix = true;                            
        create_and_send_CAN_from_GPS(gps_data);                              // create CAN message from GPS data
    }
    else
    {
    	g_tel_diagnostics.gps_fix = false;
    	osDelay(GPS_WAIT_MSG_DELAY);
    }
}

/**
 * @brief Receives GPS data and sends it as CAN message
 * @return void
*/
void GPS_delayed_rx_and_tx_as_CAN() {
    uint8_t receive_buffer[GPS_RCV_BUFFER_SIZE];
    GPS gps_data;
    GPS_msg_t gps_message;
    uint8_t gps_buffer[GPS_MESSAGE_LEN] = {UNINITIALIZED};

    read_in_NMEA_data(&gps_data, receive_buffer);           // read in NMEA data into gps struct
    check_GPS_fix_for_send(&gps_data);                      // check GPS fix and send GPS data as CAN message if there is a fix
}
