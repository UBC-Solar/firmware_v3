#include <time.h>
#include "imu.h"
#include "usart.h"
#include "sd_logger.h"

extern FIL* logfile;

/**
 * @brief Transmits IMU data over UART.
 *
 * This function transmits IMU data over UART using the specified parameters.
 *
 * @param current_timestamp The current timestamp.
 * @param imu_data The IMU data to be transmitted.
 * @param imu_type The type of IMU.
 * @param dimension The dimension of the IMU data.
 */
void transmit_imu_data(time_t current_timestamp, uint8_t* imu_data, uint8_t imu_type, uint8_t dimension)
{
    uint8_t imu_buffer[IMU_MESSAGE_LEN] = {0};

    /* Timestamp */
    for (uint8_t i = 0; i < 8; i++) {
      imu_buffer[i] = TIMESTAMP_BYTE(i, current_timestamp);
    }

    /* IMU ID */
    imu_buffer[8] = '@';

    /* IMU type and dimension */
    imu_buffer[9] = imu_type;
    imu_buffer[10] = dimension;

    /* IMU data */
    for (int i = 0; i < 4; i++) {
	    imu_buffer[11 + i] = imu_data[i];
    }

    /* New line and carriage return */
    imu_buffer[15] = '\r';
    imu_buffer[16] = '\n';

    HAL_UART_Transmit(&huart1, imu_buffer, sizeof(imu_buffer), 1000);

    sd_append(logfile, imu_buffer);

}
