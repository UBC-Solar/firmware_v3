/**
 * @file CAN_comms.h
 * 
 * @brief CAN communications layer library header.
 * This header contains user facing functions declarations and structs for the CAN communications layer.
 * 
 * @attention See README for details on how to implement this library.
 */
#ifndef CAN_COMMS_H
#define CAN_COMMS_H


#include <stdint.h>
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "stm32f1xx_hal.h"
#include "task.h"


/* Defines */
#define CAN_DATA_SIZE 8


/* Typedefs */
/**
 * @brief CAN communications Rx message struct
 * This struct contains the CAN message header and data.
 * 
 * @param header: CAN Rx header struct
 * @param data: Array of data bytes
 */
typedef struct 
{
  CAN_RxHeaderTypeDef header;
  uint8_t data[CAN_DATA_SIZE];  // TODO: Dynamically change CAN data size based on DLC
} CAN_comms_Rx_msg_t;


/**
 * @brief CAN communications Tx message struct 
 * This struct contains the CAN message header and data.
 * 
 * @param header: CAN Tx header struct
 * @param data: Array of data bytes
 */
typedef struct {
  CAN_TxHeaderTypeDef header;
  uint8_t data[CAN_DATA_SIZE];  // TODO: Dynamically change CAN data size based on DLC
} CAN_comms_Tx_msg_t;


/**
 * @brief CAN communications configuration struct
 * This struct contains the configuration for the CAN communications layer.
 * 
 * @param hcan: Pointer to the CAN_HandleTypeDef struct
 * @param CAN_Filter: CAN filter struct
 * @param CAN_comms_Rx_callback: A function pointer to the callback function that is called when a CAN message is ready to be parsed.
 */
typedef struct {
    CAN_HandleTypeDef* hcan;

    CAN_FilterTypeDef CAN_Filter;

    void (*CAN_comms_Rx_callback)(CAN_comms_Rx_msg_t*);
} CAN_comms_config_t;

/*
* @brief CAN communications diagnostic struct
*
* @param dropped_rx_msg: Count of dropped received CAN messages.Happens when rx buffer overflows.
* @param comms_init_error: indicates if comms has failed to init.
* @param rx_queue_count: the number of can messages in the RX queue
* @param success_rx: number of successful CAN received messages
* @param success_tx: number of successful CAN transmissions
* @param hal_failure_tx: number of failed CAN transmissions
* @param hal_failure_rx: number of failed CAN receives
*/
typedef struct {
   uint32_t dropped_rx_msg;
   uint8_t comms_init_error;
   uint8_t rx_queue_count;
   uint32_t success_rx;
   uint32_t success_tx;
   uint32_t hal_failure_tx;
   uint32_t hal_failure_rx;

} CAN_comms_diagnostics_t;

/* Public function declarations */
void CAN_comms_init(CAN_comms_config_t* config);
void CAN_comms_Add_Tx_message(CAN_comms_Tx_msg_t* CAN_comms_Tx_msg);
void CAN_comms_Add_Tx_messageISR(CAN_comms_Tx_msg_t* CAN_comms_Tx_msg);
void CAN_comms_get_diagnostic(CAN_comms_diagnostics_t* diagnostic);

#endif // CAN_COMMS_H
