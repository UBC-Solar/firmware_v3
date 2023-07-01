#include "i2c.h"
#include "usart.h"

enum GyroType {
    GYRO_X,
    GYRO_Y,
    GYRO_Z
};

enum AccelType {
    ACCEL_X,
    ACCEL_Y,
    ACCEL_Z
};

typedef struct {
  uint8_t imu_type;
  uint8_t dimension;
  uint8_t data[4];
} IMU_msg_t;

void initIMU(void);
float gyro(enum GyroType type);
float accel(enum AccelType type);

#define DEVICE_ADDRESS ((0x6A)<<1)

#define UART_TIMEOUT 1000

float gyro(enum GyroType type)
{

    int16_t gyro;
    uint8_t OUT_H_G, OUT_L_G;

    // goes sequential
    uint16_t addL = 0x22 + (type * 2);
    uint16_t addH = 0x23 + (type * 2);

    HAL_I2C_Mem_Read(&hi2c1, DEVICE_ADDRESS, addL, 1, &OUT_L_G, 1, 100);
    HAL_I2C_Mem_Read(&hi2c1, DEVICE_ADDRESS, addH, 1, &OUT_H_G, 1, 100);

    // The value is expressed as a 16-bit word in two’s complement.
    gyro = (OUT_H_G << 8) | (OUT_L_G);

    gyro = (float) gyro / 8.75; // datasheet pg10

    return (float) gyro * 0.02;
}

float accel(enum AccelType type)
{

    int16_t accel;
    uint8_t OUT_H_A, OUT_L_A;

    // goes sequential
    uint16_t addL = 0x28 + (type * 2);
    uint16_t addH = 0x29 + (type * 2);

    HAL_I2C_Mem_Read(&hi2c1, DEVICE_ADDRESS, addL, 1, &OUT_L_A, 1, 100);
    HAL_I2C_Mem_Read(&hi2c1, DEVICE_ADDRESS, addH, 1, &OUT_H_A, 1, 100);

    // The value is expressed as a 16-bit word in two’s complement.
    accel = (OUT_H_A << 8) | (OUT_L_A);

    accel = (float) accel * 0.061; // datasheet pg10

    return (float) accel;
}

/*
    Set up for gyro/accel modes
*/
void initIMU(void)
{
    uint8_t data;

    data = 0x80; // 0b10000000
    HAL_I2C_Mem_Write(&hi2c1, DEVICE_ADDRESS, 0x10, 1, &data, 1, 100);

    data = 0x80; // 0b10000000
    HAL_I2C_Mem_Write(&hi2c1, DEVICE_ADDRESS, 0x11, 1, &data, 1, 100);

    data = 0x04; // 0b00000100
    HAL_I2C_Mem_Write(&hi2c1, DEVICE_ADDRESS, 0x12, 1, &data, 1, 100);
}
