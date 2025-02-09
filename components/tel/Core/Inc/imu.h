/**
 *  @file imu.h
 *  @brief header file for imu.c. 
 * 
 */


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __IMU_H__
#define __IMU_H__


#define IMU_ADDRESS (0x6B << 1)

#define OUTX_L_G 0x22 //Angular rate x-axis register address lower 8 bits
#define OUTX_H_G 0x23 //Angular rate x-axis register address upper 8 bits
#define OUTY_L_G 0x24 //Angular rate y-axis register address
#define OUTY_H_G 0x25 //Angular rate y-axis register address
#define OUTZ_L_G 0x26 //Angular rate z-axis register address
#define OUTZ_H_G 0x27 //Angular rate z-axis register address

#define OUTX_L_A 0x28 //Linear Acceleration x-axis register address lower 8 bits
#define OUTX_H_A 0x29 //Linear Acceleration x-axis register address upper 8 bits
#define OUTY_L_A 0x2A //Linear Acceleration y-axis register address
#define OUTY_H_A 0x2B //Linear Acceleration y-axis register address
#define OUTZ_L_A 0x2C //Linear Acceleration z-axis register address
#define OUTZ_H_A 0x2D //Linear Acceleration z-axis register address

#define CTRL_1 0x10 //Address of CTRL1 register (Sets ODR for linear accel)
#define CTRL_2 0x11 //Address of CTRL1 register (Sets ODR for angular rate)

#define CTRL_8 0x17 //Address of CTRL8 register (Sets full scale range for linear accel)
#define CTRL_6 0x15 //Address of CTRL6 register (Sets full scale range for angular rate)

void imu_task();

#endif /* __RADIO_H__ */
