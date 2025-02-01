/******************************************************************************
* @file    gps.c
* @brief   Sets g_gps_read_okay to true or false based on I2C reading
******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "gps.h"
#include "nmea_parse.h"

#define GPS_DEVICE_ADDRESS ((0x42)<<1)

bool g_gps_read_okay = false;
uint8_t g_gps_data[GPS_MESSAGE_LEN];
char gps_parse_data[GPS_MESSAGE_LEN];

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
 * @brief Continually tries to get a fix. Only returns once a fix is obtained
 * @param receive_buffer The buffer to store the received data
 * @return bool True if a fix was obtained, false otherwise
 */
bool read_i2c_gps_module(uint8_t* receive_buffer)
{
	  g_gps_read_okay = false;
    if(HAL_I2C_Master_Receive_IT(&hi2c1, GPS_DEVICE_ADDRESS, receive_buffer, GPS_MESSAGE_LEN) == HAL_OK)
    {
        // Set status to true if i2c read was successful
        g_gps_read_okay = true;
    }

    return g_gps_read_okay;
}

/**
 * @brief Reads the GPS data and confirms if it is read to be parsed into gps_data 
 */
void gps_task()
{
  osDelay(500);

    // uint8_t msg[20] = "New GPS read\r\n\n";
    // HAL_UART_Transmit(&huart5, msg, 20, 100);

    memset(g_gps_data, 0, GPS_MESSAGE_LEN);

    read_i2c_gps_module(g_gps_data);

    osDelay(500);

   if(g_gps_read_okay)
   {
    //  HAL_UART_Transmit(&huart5, g_gps_data, GPS_MESSAGE_LEN, 100);

     GPS gps_data = {0};

     nmea_parse(&gps_data, g_gps_data);

     CAN_tx_gps_data_msg(&gps_data);

     g_gps_read_okay = false;
   }
  //  else
  //  {
  //    strncpy(g_gps_data, "GPS not connected\r\n", GPS_MESSAGE_LEN);
  //    HAL_UART_Transmit(&huart5, g_gps_data, GPS_MESSAGE_LEN, 100);
  //  }

    HAL_GPIO_TogglePin(USER_LED_GPIO_Port, USER_LED_Pin);
}