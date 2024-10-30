#ifndef INC_CANBUSLOAD_H_
#define INC_CANBUSLOAD_H_

#include "can.h"

#define CANLOAD_BAUD_RATE 500000

float CANLOAD_get_bus_load();
void CANLOAD_update_sliding_window();
void CANLOAD_calculate_message_bits(uint32_t DLC);

#endif /* INC_CANBUSLOAD_H_ */
