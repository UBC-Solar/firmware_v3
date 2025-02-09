#include "main.h"
#include "i2c.h"
#include "usart.h"
#include "imu.h"
#include <stdio.h>
#include <string.h>
#include "can.h"

typedef struct {
	float accelx;
	float accely;
	float accelz;
	float gyrox;
	float gyroy;
	float gyroz;
} IMU;

void imu_task()
{
    int8_t bufx[2];
	int8_t bufy[2];
	int8_t bufz[2];
	uint8_t debug_buf[100] = {0};

	HAL_StatusTypeDef ret_accelx;
	HAL_StatusTypeDef ret_accely;
	HAL_StatusTypeDef ret_accelz;

	HAL_StatusTypeDef ret_gyrox;
	HAL_StatusTypeDef ret_gyroy;
	HAL_StatusTypeDef ret_gyroz;

    // Read OUTX_L_A and OUTX_H_A
    // Write to CTRL1 register
    bufx[0] = 0x74; // Set ODR to 30 Hz, normal mode
    ret_accelx = HAL_I2C_Mem_Write(&hi2c2, IMU_ADDRESS, CTRL_1, 1, bufx, 1, 10);
    bufy[0] = 0x02; // Set the range to +-8g (might have to change later - check if sufficient for cornering?)
    ret_accely = HAL_I2C_Mem_Write(&hi2c2, IMU_ADDRESS, CTRL_8, 1, bufy, 1, 10);

    if (ret_accelx != HAL_OK) {
        sprintf((char*)debug_buf, "Error writing CTRL1: %d\r\n", ret_accelx);
        HAL_UART_Transmit(&huart5, debug_buf, sizeof(debug_buf), 10);
    }

    if (ret_accely != HAL_OK) {
        sprintf((char*)debug_buf, "Error writing CTRL8: %d\r\n", ret_accely);
        HAL_UART_Transmit(&huart5, debug_buf, sizeof(debug_buf), 10);
    }

    // Write to CTRL2 register
    bufx[0] = 0x54; // Set ODR to 30HZ, low-power mode;
    ret_accelx = HAL_I2C_Mem_Write(&hi2c2, IMU_ADDRESS, CTRL_2, 1, bufx, 1, 10);
    bufy[0] = 0x01; // Set the scale to +-250 degrees per sec (dps) (might have to change later - check if sufficient for cornering?)
    ret_accely = HAL_I2C_Mem_Write(&hi2c2, IMU_ADDRESS, CTRL_6, 1, bufy, 1, 10);

    if (ret_accelx != HAL_OK) {
        sprintf((char*)debug_buf, "Error writing CTRL2: %d\r\n", ret_accelx);
        HAL_UART_Transmit(&huart5, debug_buf, sizeof(debug_buf), 10);
    }

    if (ret_accely != HAL_OK) {
        sprintf((char*)debug_buf, "Error writing CTRL6: %d\r\n", ret_accely);
        HAL_UART_Transmit(&huart5, debug_buf, sizeof(debug_buf), 10);
    }



    for (;;)
    {
    	//Reading Accelerometer data
    	IMU imu_data;

		ret_accelx = HAL_I2C_Mem_Read(&hi2c2, IMU_ADDRESS, OUTX_L_A, 1, bufx, 2, 10);
		ret_accely = HAL_I2C_Mem_Read(&hi2c2, IMU_ADDRESS, OUTY_L_A, 1, bufy, 2, 10);

		if (ret_accelx != HAL_OK || ret_accely != HAL_OK) {
			sprintf((char*)debug_buf, "Error reading accel: %d\r\n", ret_accelx);
			HAL_UART_Transmit(&huart5, debug_buf, sizeof(debug_buf), 10);
		}

		else {
			int16_t Accel_X_RAW = (int16_t)(bufx[1] << 8 | bufx[0]);
			int16_t Accel_Y_RAW = (int16_t)(bufy[1] << 8 | bufy[0]);

			float accel_x = (float)Accel_X_RAW * 0.244; // Convert to mg (Multiply by Datasheet value of 0.244)
			float accel_y = (float)Accel_Y_RAW * 0.244; // Convert to mg

			imu_data.accelx = accel_x;
			imu_data.accely = accel_y;
		}


		ret_accelz = HAL_I2C_Mem_Read(&hi2c2, IMU_ADDRESS, OUTZ_L_A, 1, bufz, 2, 10);
		ret_gyrox = HAL_I2C_Mem_Read(&hi2c2, IMU_ADDRESS, OUTX_L_G, 1, bufx, 2, 10);

		if (ret_accely != HAL_OK || ret_gyrox != HAL_OK) {
			sprintf((char*)debug_buf, "Error reading accel: %d\r\n", ret_accelx);
			HAL_UART_Transmit(&huart5, debug_buf, sizeof(debug_buf), 10);
		}

		else {
			int16_t Accel_Z_RAW = (int16_t)(bufz[1] << 8 | bufz[0]);
			int16_t Gyro_X_RAW = (int16_t)(bufx[1] << 8 | bufx[0]);


			float accel_z = (float)Accel_Z_RAW * 0.244; // Convert to mg
			float gyro_x = Gyro_X_RAW * 8.75; // Convert to milli degrees per sec (mdps) (Multiply by Datasheet value of 8.75)

			imu_data.accelz = accel_z;
			imu_data.gyrox = gyro_x;
		}


		ret_gyroy = HAL_I2C_Mem_Read(&hi2c2, IMU_ADDRESS, OUTY_L_G, 1, bufy, 2, 10);
		ret_gyroz = HAL_I2C_Mem_Read(&hi2c2, IMU_ADDRESS, OUTZ_L_G, 1, bufz, 2, 10);

		if (ret_gyroy != HAL_OK || ret_gyroz != HAL_OK) {
			sprintf((char*)debug_buf, "Error reading gyro: %d\r\n", ret_accelx);
			HAL_UART_Transmit(&huart5, debug_buf, sizeof(debug_buf), 10);
		}

		else {  

			int16_t Gyro_Y_RAW = (int16_t)(bufy[1] << 8 | bufy[0]);
			int16_t Gyro_Z_RAW = (int16_t)(bufz[1] << 8 | bufz[0]);


			float gyro_y = Gyro_Y_RAW * 8.75; // Convert to mdps
			float gyro_z = Gyro_Z_RAW * 8.75; // Convert to mdps

			imu_data.gyroy = gyro_y;
			imu_data.gyroz = gyro_z;
		}

		osDelay(3);
		CAN_tx_ag_x_msg(imu_data.accelx, imu_data.gyrox);
		osDelay(3);
		CAN_tx_ag_x_msg(imu_data.accely, imu_data.gyroy);
		osDelay(3);
		CAN_tx_ag_x_msg(imu_data.accelz, imu_data.gyroz);

        osDelay(100);
	}
}
