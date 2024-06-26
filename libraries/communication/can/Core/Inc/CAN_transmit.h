#include <stdint.h>
#include "stm32f1xx_hal_can.h"

void CANTX_send(CAN_TxHeaderTypeDef tx_header, uint8_t tx_data[], CAN_HandleTypeDef *can_handle, uint32_t mailbox, void (*Error_Handler)());
