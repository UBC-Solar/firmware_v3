/*
 * canbusload.h
 *
 *  Created on: Oct 17, 2024
 *      Author: makac
 */

#ifndef INC_CANBUSLOAD_H_
#define INC_CANBUSLOAD_H_

#include "can.h"

#define CANLOAD_BIT_RATE 500000
#ifndef CANLOAD_WINDOW_SIZE
    #define CANLOAD_WINDOW_SIZE 5
#endif

#define CANLOAD_SOF_BITS 1
#define CANLOAD_STANDARD_ID_BITS 11
#define CANLOAD_EXTENDED_ID_BITS 29
#define CANLOAD_SRR_BITS 1
#define CANLOAD_RESERVED_BIT_R1 1
#define CANLOAD_RTR_BITS 1
#define CANLOAD_IDE_BITS 1
#define CANLOAD_RESERVED_BIT_R0 1
#define CANLOAD_DLC_BITS 4
#define CANLOAD_CRC_BITS 15
#define CANLOAD_CRC_DELIMITER_BITS 1
#define CANLOAD_ACK_SLOT_BITS 1
#define CANLOAD_ACK_DELIMITER_BITS 1
#define CANLOAD_EOF_BITS 7
#define CANLOAD_IFS_BITS 3

extern uint32_t can_total_bits;

float CANLOAD_get_bus_load();
uint32_t CANLOAD_calculate_message_bits(CAN_msg_t *msg);

#endif /* INC_CANBUSLOAD_H_ */
