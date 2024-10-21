#ifndef CAN_COMMS_H
#define CAN_COMMS_H

#include <stdint.h>

#include "can.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "stm32f1xx_hal.h"
#include "task.h"

/* Defines */
#define CAN_DATA_SIZE 8

/* Typedefs */
typedef struct {
  CAN_RxHeaderTypeDef header;
  uint8_t data[CAN_DATA_SIZE];
} CAN_comms_Rx_msg_t;

typedef struct {
  CAN_TxHeaderTypeDef header;
  uint8_t data[CAN_DATA_SIZE];
} CAN_comms_Tx_msg_t;


/**
 * @brief CAN communications configuration struct
 * This struct contains the configuration for the CAN communications layer.
 * 
 * @param hcan: Pointer to the CAN_HandleTypeDef struct
 * @param RX_FIFO: The RX FIFO to use
 * @param CAN_comms_Rx_callback: A function pointer to the callback function that is called when a CAN message is ready to be parsed.
 */
typedef struct {
    CAN_HandleTypeDef* hcan;

    uint8_t RX_FIFO;

    void (*CAN_comms_Rx_callback)(CAN_comms_Rx_msg_t);
} CAN_comms_config_t;


/* Public function definitions */
void CAN_comms_init(CAN_comms_config_t* config);


#endif // CAN_COMMS_H
