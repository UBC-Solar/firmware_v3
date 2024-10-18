#ifndef CAN_COMMS_H
#define CAN_COMMS_H

#include <stdint.h>

#include "can.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "stm32f1xx_hal.h"
#include "task.h"




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


/* Public function definitions */
void CAN_comms_init(CAN_comms_config_t* config);
void ISR_CAN_comms_Rx();



#endif // CAN_COMMS_H
