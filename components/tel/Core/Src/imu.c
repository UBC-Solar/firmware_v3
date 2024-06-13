
/**
 *  @file imu.c
 *  @brief Defines functions for setting accelerometer and gyroscope data as well as packaging IMU as CAN and sending it on the bus and radio.
 *
 *  @date 2023/03/18
 */

#include "imu.h"

/**
 * @brief Combines two bytes into a 16-bit integer.
 * @param rawData The raw data.
 * @param startIndex The start index.
 * @return The combined bytes.
*/
static uint16_t combine_bytes(uint8_t* rawData, uint8_t startIndex) {
  return (uint16_t)(rawData[startIndex] << NUM_BITS_IN_BYTE_SHIFT | rawData[startIndex + 1]);
}


/**
 * @brief Sets the data based on raw data from i2c.
 * @param x The x-axis data.
 * @param y The y-axis data.
 * @param z The z-axis data.
 * @param raw_data The raw data.
 * @param scaler The scaler to correct raw data.
 * @return void
*/
static void set_data(union Utils_FloatBytes_t* x, union Utils_FloatBytes_t* y, union Utils_FloatBytes_t* z, uint8_t* raw_data, double scaler) {
    uint16_t raw_x = combine_bytes(raw_data, RAW_X_START_BYTE);
    uint16_t raw_y = combine_bytes(raw_data, RAW_Y_START_BYTE);
    uint16_t raw_z = combine_bytes(raw_data, RAW_Z_START_BYTE);

    x->float_value = raw_x / scaler;
    y->float_value = raw_y / scaler;
    z->float_value = raw_z / scaler;
} 


/**
 * @brief Sets the accelerometer data.
 * @param ax_x The x-axis accelerometer data.
 * @param ax_y The y-axis accelerometer data.
 * @param ax_z The z-axis accelerometer data.
 * @param imu_status The status of the IMU.
 */
static void set_accelorator_data(union Utils_FloatBytes_t* ax_x, union Utils_FloatBytes_t* ax_y, union Utils_FloatBytes_t* ax_z, HAL_StatusTypeDef* imu_status) {
    /* Read accelerator data */
    uint8_t accel_data[NUM_ACCEL_BYTES];
    UTILS_zeroArray(accel_data, NUM_ACCEL_BYTES);
    *imu_status |= HAL_I2C_Mem_Read(&hi2c2, IMU_DEVICE_ADDRESS, ACCEL_XOUT_H_REG, MEM_ADDRESS_SIZE, accel_data, NUM_ACCEL_BYTES, READ_TIMEOUT);
    set_data(ax_x, ax_y, ax_z, accel_data, ACCEL_SENSITIVITY);
}


/**
 * @brief Sets the gyroscope data.
 * @param gy_x The x-axis gyroscope data.
 * @param gy_y The y-axis gyroscope data.
 * @param gy_z The z-axis gyroscope data.
 * @param imu_status The status of the IMU.
*/
static void set_gyro_data(union Utils_FloatBytes_t* gy_x, union Utils_FloatBytes_t* gy_y, union Utils_FloatBytes_t* gy_z, HAL_StatusTypeDef* imu_status) {
    /* Read gyroscope data */
    uint8_t gyro_data[NUM_GYRO_BYTES];
    UTILS_zeroArray(gyro_data, NUM_GYRO_BYTES);
    *imu_status |= HAL_I2C_Mem_Read(&hi2c2, IMU_DEVICE_ADDRESS, GYRO_XOUT_H_REG, MEM_ADDRESS_SIZE, gyro_data, NUM_GYRO_BYTES, READ_TIMEOUT);
    set_data(gy_x, gy_y, gy_z, gyro_data, GYRO_SENSITIVITY);
}

static void set_data_as_CAN_msg(CAN_Radio_msg_t *x_data, CAN_Radio_msg_t *y_data, CAN_Radio_msg_t *z_data, 
                             union Utils_FloatBytes_t* ax_x, union Utils_FloatBytes_t* ax_y, union Utils_FloatBytes_t* ax_z, 
                             union Utils_FloatBytes_t* gy_x, union Utils_FloatBytes_t* gy_y, union Utils_FloatBytes_t* gy_z) 
{
    /* Get current epoch Time Stamp */
    union Utils_DoubleBytes_t current_timestamp;
    current_timestamp.double_value = get_current_timestamp();

    /* Set headers */
    z_data->header = IMU_z_axis_header;
    y_data->header = IMU_y_axis_header;
    x_data->header = IMU_x_axis_header;

    /* Assign the timestamp */
    x_data->timestamp = current_timestamp;
    y_data->timestamp = current_timestamp;
    z_data->timestamp = current_timestamp;

    for (int i = 0; i < GYRO_DATA_START_BYTE; i++) {
        // Z-axis data
        z_data->data[i] = ax_z->bytes[i];
        z_data->data[GYRO_DATA_START_BYTE + i] = gy_z->bytes[i];
        
        // Y-axis data
        y_data->data[i] = ax_y->bytes[i];
        y_data->data[GYRO_DATA_START_BYTE + i] = gy_y->bytes[i];

        // X-axis data
        x_data->data[i] = ax_x->bytes[i];
        x_data->data[GYRO_DATA_START_BYTE + i] = gy_x->bytes[i];
    }
}

static void create_and_send_CAN_msg(CAN_Radio_msg_t *x_data, CAN_Radio_msg_t *y_data, CAN_Radio_msg_t *z_data, 
                             union Utils_FloatBytes_t* ax_x, union Utils_FloatBytes_t* ax_y, union Utils_FloatBytes_t* ax_z, 
                             union Utils_FloatBytes_t* gy_x, union Utils_FloatBytes_t* gy_y, union Utils_FloatBytes_t* gy_z)
{
    set_data_as_CAN_msg(x_data, y_data, z_data, 
                            ax_x, ax_y, ax_z, 
                            gy_x, gy_y, gy_z);

    /* Transmit the messages */
    CAN_radio_and_bus_transmit(&hcan, x_data, &can_mailbox);
    CAN_radio_and_bus_transmit(&hcan, y_data, &can_mailbox);
    CAN_radio_and_bus_transmit(&hcan, z_data, &can_mailbox);
}

void IMU_send_as_CAN_msg_single_delay(HAL_StatusTypeDef* imu_status ) {
    union Utils_FloatBytes_t ax_x, ax_y, ax_z, gy_x, gy_y, gy_z;
    CAN_Radio_msg_t x_axis_data, y_axis_data, z_axis_data;

    set_accelorator_data(&ax_x, &ax_y, &ax_z, imu_status);
    osDelay(IMU_SINGLE_DELAY);
    set_gyro_data(&gy_x, &gy_y, &gy_z, imu_status);

    if (*imu_status != HAL_OK) {                        // If imu could not be read then dont send messages
        return;
    }

    create_and_send_CAN_msg(&x_axis_data, &y_axis_data, &z_axis_data, 
                                &ax_x, &ax_y, &ax_z, 
                                &gy_x, &gy_y, &gy_z);
}
