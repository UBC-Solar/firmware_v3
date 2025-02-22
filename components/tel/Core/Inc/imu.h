/**
 *  @file imu.h
 *  @brief Header file for IMU communication.
 *
 *  This file contains macros for IMU register addresses and function declarations
 *  for interfacing with the IMU.
 */

#ifndef __IMU_H__
#define __IMU_H__

#ifdef __cplusplus
extern "C" {
#endif

/* IMU I2C Address */
#define IMU_ADDRESS (0x6B << 1)  ///< IMU I2C device address (shifted for STM32 HAL)

/* Gyroscope Output Registers */
#define OUTX_L_G  0x22 // Gyro X-axis (low byte)
#define OUTY_L_G  0x24 // Gyro Y-axis (low byte)
#define OUTZ_L_G  0x26 // Gyro Z-axis (low byte)

/* Accelerometer Output Registers */
#define OUTX_L_A  0x28 // Accel X-axis (low byte)
#define OUTY_L_A  0x2A // Accel Y-axis (low byte)
#define OUTZ_L_A  0x2C // Accel Z-axis (low byte)

/* Control Registers */
#define CTRL_1    0x10 // Control register for linear acceleration ODR
#define CTRL_2    0x11 // Control register for angular rate ODR
#define CTRL_6    0x15 // Control register for gyroscope full-scale range
#define CTRL_8    0x17 // Control register for accelerometer full-scale range

/**
 * @brief Task function for getting IMU data and transmitting over CAN.
 */
void imu_task(void);

void static CAN_tx_ag_x_msg(float accel_x, float gyros_x);
void static CAN_tx_ag_y_msg(float accel_y, float gyro_y);
void static CAN_tx_ag_z_msg(float accel_z, float gyro_z);

#ifdef __cplusplus
}
#endif

#endif /* __IMU_H__ */
