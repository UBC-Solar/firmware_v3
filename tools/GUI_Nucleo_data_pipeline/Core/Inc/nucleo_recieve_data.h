#ifndef INC_NUCLEO_RECIEVE_DATA_H_
#define INC_NUCLEO_RECIEVE_DATA_H_

#include "main.h"   //nucleo project main header file
#include <limits.h> //ensure these last four aren't included anywhere else
#include <math.h>
#include <stdlib.h>
#include <string.h>

typedef struct ID_table_t
{
    int CAN_ID;
    char *datatype;
} ID_table_t;

#define NEWLINE_CHAR "\n"
#define CAN_BYTES_SENT 8
#define TIMEOUT 100
#define TABLE_SIZE 5

#endif
