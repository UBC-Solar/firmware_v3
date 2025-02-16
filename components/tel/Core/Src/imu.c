#include "main.h"
#include "i2c.h"
#include "imu.h"
#include "can.h"
#include <stdio.h>
#include <string.h>
#include "cmsis_os.h"

/* Define the CAN message length for IMU messages */
#define IMU_CAN_MESSAGE_LENGTH 8

/* Define the CAN message IDs for each IMU axes */
#define IMU_AG_X_CAN_MESSAGE_ID 0x752
#define IMU_AG_Y_CAN_MESSAGE_ID 0x753
#define IMU_AG_Z_CAN_MESSAGE_ID 0x754

//Struct for the IMU data
typedef struct {
	float accelx;
	float accely;
	float accelz;
	float gyrox;
	float gyroy;
	float gyroz;
} IMU;

/* Static CAN header definitions */
static CAN_TxHeaderTypeDef imu_ag_x = {
    .StdId = IMU_AG_X_CAN_MESSAGE_ID,
    .ExtId = 0x0000,
    .IDE   = CAN_ID_STD,
    .RTR   = CAN_RTR_DATA,
    .DLC   = IMU_CAN_MESSAGE_LENGTH
};

static CAN_TxHeaderTypeDef imu_ag_y = {
    .StdId = IMU_AG_Y_CAN_MESSAGE_ID,
    .ExtId = 0x0000,
    .IDE   = CAN_ID_STD,
    .RTR   = CAN_RTR_DATA,
    .DLC   = IMU_CAN_MESSAGE_LENGTH
};

static CAN_TxHeaderTypeDef imu_ag_z = {
    .StdId = IMU_AG_Z_CAN_MESSAGE_ID,
    .ExtId = 0x0000,
    .IDE   = CAN_ID_STD,
    .RTR   = CAN_RTR_DATA,
    .DLC   = IMU_CAN_MESSAGE_LENGTH
};

/* A union for converting between float and a 4-byte array */
static union {
    float value;
    int8_t bytes[4];
} float_bytes;

/**
 * @brief Sends the combined accel_x and gyro_x values over CAN.
 * @param accel_x Acceleration value for the x-axis.
 * @param gyro_x  Gyroscope value for the x-axis.
 */
static void CAN_tx_ag_x_msg(float accel_x, float gyro_x)
{
    float_bytes.value = accel_x;
    CAN_comms_Tx_msg_t msg = { .header = imu_ag_x };

    for (int i = 0; i < 4; i++)
    {
        msg.data[i] = float_bytes.bytes[i];
    }

    float_bytes.value = gyro_x;
    for (int i = 0; i < 4; i++)
    {
        msg.data[i + 4] = float_bytes.bytes[i];
    }

    CAN_comms_Add_Tx_message(&msg);
    RADIO_filter_and_queue_msg_tx(&msg);
}

/**
 * @brief Sends the combined accel_y and gyro_y values over CAN.
 * @param accel_y Acceleration value for the y-axis.
 * @param gyro_y  Gyroscope value for the y-axis.
 */
static void CAN_tx_ag_y_msg(float accel_y, float gyro_y)
{
    float_bytes.value = accel_y;
    CAN_comms_Tx_msg_t msg = { .header = imu_ag_y };

    for (int i = 0; i < 4; i++)
    {
        msg.data[i] = float_bytes.bytes[i];
    }

    float_bytes.value = gyro_y;
    for (int i = 0; i < 4; i++)
    {
        msg.data[i + 4] = float_bytes.bytes[i];
    }

    CAN_comms_Add_Tx_message(&msg);
    RADIO_filter_and_queue_msg_tx(&msg);
}

/**
 * @brief Sends the combined accel_z and gyro_z values over CAN.
 * @param accel_z Acceleration value for the z-axis.
 * @param gyro_z  Gyroscope value for
 * the z-axis.
 */
static void CAN_tx_ag_z_msg(float accel_z, float gyro_z)
{
    float_bytes.value = accel_z;
    CAN_comms_Tx_msg_t msg = { .header = imu_ag_z };

    for (int i = 0; i < 4; i++)
    {
        msg.data[i] = float_bytes.bytes[i];
    }

    float_bytes.value = gyro_z;
    for (int i = 0; i < 4; i++)
    {
        msg.data[i + 4] = float_bytes.bytes[i];
    }

    CAN_comms_Add_Tx_message(&msg);
    RADIO_filter_and_queue_msg_tx(&msg);
}


/**
 * @brief Write a single byte to an IMU register.
 * @param reg The register address.
 * @param value The value to write.
 * @return HAL status.
 */
static HAL_StatusTypeDef write_imu_register (uint16_t reg, uint8_t val){
    return HAL_I2C_Mem_Write(&hi2c2, IMU_ADDRESS, reg, 1, &val, 1, 10);
}

/**
 * @brief Read two bytes from an IMU register and return the 16-bit value.
 *        If the read fails, 0 is returned.
 * @param reg The starting register address.
 * @return The 16-bit value read from the sensor.
 */
static uint16_t read_imu_register (uint16_t reg){
	uint8_t data[2];
	if(HAL_I2C_Mem_Read(&hi2c2, IMU_ADDRESS, reg, 1, data, 2, 10) != HAL_OK){
		return 0;
	}

	return (int16_t)(data[1] << 8 | data[0]);
}


/**
 * @brief Task to read IMU data and transmit it over the CAN bus.
 */
void imu_task()
{
	//Writing to CTRL_1 register, sets ODR to 30 Hz in normal mode and the range to +-8g in CTRL_8 register
    if(write_imu_register(CTRL_1, 0x74) != HAL_OK || write_imu_register(CTRL_8, 0x02) != HAL_OK){
    	return 0;
    }

    //Writing to CTRL_2 register, sets ODR to 30 Hz in low power mode and the scale to +- 250 dps in CTRL_6 register
    if(write_imu_register(CTRL_2, 0x54) != HAL_OK || write_imu_register(CTRL_6, 0x01) != HAL_OK){
        return 0;
    }

    for (;;)
    {
    	IMU imu_data;

    	//Reading Accelerometer data
    	int16_t Accel_X_RAW = read_imu_register(OUTX_L_A);
    	int16_t Accel_Y_RAW = read_imu_register(OUTY_L_A);
    	int16_t Accel_Z_RAW = read_imu_register(OUTZ_L_A);

    	//Converting raw values to mg according to datasheet, where mg=9.81 * 10^-3 m/s^2.
    	imu_data.accelx = (float)Accel_X_RAW * 0.244; // Convert to mg
    	imu_data.accely = (float)Accel_Y_RAW * 0.244; // Convert to mg
    	imu_data.accelz = (float)Accel_Z_RAW * 0.244; // Convert to mg

    	//Reading Gyroscopic data
    	int16_t Gyro_X_RAW = read_imu_register(OUTX_L_G);
		int16_t Gyro_Y_RAW = read_imu_register(OUTY_L_G);
		int16_t Gyro_Z_RAW = read_imu_register(OUTZ_L_G);

		//Converting raw values to milli degrees per sec (mdps) according to datasheet.
		imu_data.gyrox = (float)Gyro_X_RAW * 8.75; // Convert to mdps
		imu_data.gyroy = (float)Gyro_Y_RAW * 8.75; // Convert to mdps
		imu_data.gyroz = (float)Gyro_Z_RAW * 8.75; // Convert to mdps

		//Sending message over to the CAN bus
		osDelay(3);
		CAN_tx_ag_x_msg(imu_data.accelx, imu_data.gyrox);
		osDelay(3);
		CAN_tx_ag_y_msg(imu_data.accely, imu_data.gyroy);
		osDelay(3);
		CAN_tx_ag_z_msg(imu_data.accelz, imu_data.gyroz);
        osDelay(100);
	}
}
