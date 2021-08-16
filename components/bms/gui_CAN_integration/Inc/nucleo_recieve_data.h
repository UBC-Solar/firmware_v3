#ifndef INC_NUCLEO_RECIEVE_DATA_H_
#define INC_NUCLEO_RECIEVE_DATA_H_

#ifndef MASTERBOARD_INCLUDES
#include "masterboard_data_collection.h"
#endif

CDC_Transmit_FS(NEWLINE, strlen(data_char));
static void CAN_config(void);

#endif
