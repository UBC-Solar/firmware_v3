#ifndef CAN_COMMS_H
#define CAN_COMMS_H

#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "can.h"
#include "stm32f1xx_hal.h"
#include "cmsis_os2.h"


#define CAN_DATA_SIZE 8
/* Typedefs */
typedef struct {
  CAN_RxHeaderTypeDef header;
  uint8_t data[8];
} CAN_comms_Rx_msg_t;

/**
 * @brief 
 * 
 */
typedef struct {
    CAN_HandleTypeDef* hcan;
    uint8_t RX_FIFO;
    void (*CAN_comms_Rx_callback)(CAN_comms_Rx_msg_t);

} CAN_comms_config_t;

#endif // CAN_COMMS_H