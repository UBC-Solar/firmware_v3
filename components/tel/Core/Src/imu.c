#include "main.h"
#include "i2c.h"
#include "usart.h"
#include "imu.h"
#include <stdio.h>
#include <string.h>

void imu_task()
{
    int8_t bufx[2];
	int8_t bufy[2];
	int8_t bufz[2];
	uint8_t debug_buf[50];
	HAL_StatusTypeDef retx;
	HAL_StatusTypeDef rety;
	HAL_StatusTypeDef retz;

    for (;;)
    {


        // Read OUTX_L_A and OUTX_H_A
	  	// Write to CTRL1 register
		bufx[0] = 0x74; // Set ODR to 30 Hz, normal mode
		retx = HAL_I2C_Mem_Write(&hi2c2, IMU_ADDRESS, CTRL_1, 1, bufx, 1, 1000);
		bufy[0] = 0x02; // Set the range to +-8g (might have to change later - check if sufficient for cornering?)
		rety = HAL_I2C_Mem_Write(&hi2c2, IMU_ADDRESS, CTRL_8, 1, bufy, 1, 1000);

		if (retx != HAL_OK) {
			sprintf((char*)debug_buf, "Error writing CTRL1: %d\r\n", retx);
			HAL_UART_Transmit(&huart5, debug_buf, strlen((char*)debug_buf), 1000);
		}
		if (rety != HAL_OK) {
			sprintf((char*)debug_buf, "Error writing CTRL8: %d\r\n", rety);
			HAL_UART_Transmit(&huart5, debug_buf, strlen((char*)debug_buf), 1000);
		}

		retx = HAL_I2C_Mem_Read(&hi2c2, IMU_ADDRESS, OUTX_L_A, 1, bufx, 2, 1000);
		rety = HAL_I2C_Mem_Read(&hi2c2, IMU_ADDRESS, OUTY_L_A, 1, bufy, 2, 1000);
		retz = HAL_I2C_Mem_Read(&hi2c2, IMU_ADDRESS, OUTZ_L_A, 1, bufz, 2, 1000);

		if (retx != HAL_OK || rety != HAL_OK || retz != HAL_OK) {
			sprintf((char*)debug_buf, "Error reading accel: %d\r\n", retx);
			HAL_UART_Transmit(&huart5, debug_buf, strlen((char*)debug_buf), 1000);
		}


		else {
			int16_t Accel_X_RAW = (int16_t)(bufx[1] << 8 | bufx[0]);
			int16_t Accel_Y_RAW = (int16_t)(bufy[1] << 8 | bufy[0]);
			int16_t Accel_Z_RAW = (int16_t)(bufz[1] << 8 | bufz[0]);

			float accel_x = Accel_X_RAW * 0.488; // Convert to mg (Multiply by Datasheet value of 0.488)
			float accel_y = Accel_Y_RAW * 0.488; // Convert to mg
			float accel_z = Accel_Z_RAW * 0.488; // Convert to mg

			sprintf((char*)debug_buf, "Accel X: %.2f mg\t Accel Y: %.2f mg\t Accel Z: %.2f mg\r\n", accel_x, accel_y, accel_z);
			HAL_UART_Transmit(&huart5, debug_buf, strlen((char*)debug_buf), 1000);
		}

		// Write to CTRL2 register
		bufx[0] = 0x54; // Set ODR to 30HZ, low-power mode;
		retx = HAL_I2C_Mem_Write(&hi2c2, IMU_ADDRESS, CTRL_2, 1, bufx, 1, 1000);
		bufy[0] = 0x01; // Set the scale to +-500 degrees per sec (dps) (might have to change later - check if sufficient for cornering?)
		rety = HAL_I2C_Mem_Write(&hi2c2, IMU_ADDRESS, CTRL_6, 1, bufy, 1, 1000);

		if (retx != HAL_OK) {
			sprintf((char*)debug_buf, "Error writing CTRL2: %d\r\n", retx);
			HAL_UART_Transmit(&huart5, debug_buf, strlen((char*)debug_buf), 1000);
		}

		if (rety != HAL_OK) {
			sprintf((char*)debug_buf, "Error writing CTRL6: %d\r\n", rety);
			HAL_UART_Transmit(&huart5, debug_buf, strlen((char*)debug_buf), 1000);
		}

		retx = HAL_I2C_Mem_Read(&hi2c2, IMU_ADDRESS, OUTX_L_G, 1, bufx, 2, 1000);
		rety = HAL_I2C_Mem_Read(&hi2c2, IMU_ADDRESS, OUTY_L_G, 1, bufy, 2, 1000);
		retz = HAL_I2C_Mem_Read(&hi2c2, IMU_ADDRESS, OUTZ_L_G, 1, bufz, 2, 1000);

		if (retx != HAL_OK || rety != HAL_OK || retz != HAL_OK) {
			sprintf((char*)debug_buf, "Error reading gyro: %d\r\n", retx);
			HAL_UART_Transmit(&huart5, debug_buf, strlen((char*)debug_buf), 1000);
		}


		else {
			int16_t Gyro_X_RAW = (int16_t)(bufx[1] << 8 | bufx[0]);
			int16_t Gyro_Y_RAW = (int16_t)(bufy[1] << 8 | bufy[0]);
			int16_t Gyro_Z_RAW = (int16_t)(bufz[1] << 8 | bufz[0]);

			float gyro_x = Gyro_X_RAW * 8.75; // Convert to milli degrees per sec (mdps) (Multiply by Datasheet value of 8.75)
			float gyro_y = Gyro_Y_RAW * 8.75; // Convert to mdps
			float gyro_z = Gyro_Z_RAW * 8.75; // Convert to mdps

			sprintf((char*)debug_buf, "Gyro X: %.2f mdps\t Gyro Y: %.2f mdps\t Gyro Z: %.2f mdps\r\n", gyro_x, gyro_y, gyro_z);
			HAL_UART_Transmit(&huart5, debug_buf, strlen((char*)debug_buf), 1000);
			osDelay(2000);
		}

	}
}
