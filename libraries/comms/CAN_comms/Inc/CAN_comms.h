#ifndef CAN_COMMS_H
#define CAN_COMMS_H

#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "can.h"
#include "stm32f1xx_hal.h"


#define CAN_DATA_SIZE 8
#define CAN_QUEUE_SIZE 16

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
    void (*CAN_comms_handle_Rx)(CAN_comms_Rx_msg_t* CAN_comms_Rx_msg);

} CAN_comms_config_t;

#endif // CAN_COMMS_H