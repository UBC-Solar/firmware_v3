#ifndef INC_CANBUSLOAD_H_
#define INC_CANBUSLOAD_H_

#include "main.h"
#define CANLOAD_MSG_RATE 100

float CANLOAD_get_bus_load();
void CANLOAD_update_sliding_window();
void CANLOAD_calculate_message_bits(uint32_t DLC, uint32_t IDE);

#endif /* INC_CANBUSLOAD_H_ */
