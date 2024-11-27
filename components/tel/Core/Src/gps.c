#include "gps.h"

#define GPS_DEVICE_ADDRESS ((0x42)<<1)
#define GPS_I2C_TIMEOUT    (1000)
bool g_gps_read_okay = false;

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
	bool status = false;
    if(HAL_I2C_Master_Receive(&hi2c2, GPS_DEVICE_ADDRESS, receive_buffer, GPS_MESSAGE_LEN, 100) == HAL_OK)
    {
        // Set status to true if i2c read was successful
        status = true;
    }

    return status;
}
