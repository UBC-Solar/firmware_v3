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
    struct ID_table_t *next;
} ID_table_t;

//hash table functions

unsigned int hash(unsigned int id);
void init_hash_table();
char *hash_table_lookup(int id);
void hash_table_insert(ID_table_t *id_struct);

//characters GUI uses to classify type of incomming data

//these CAN IDs are placeholder values until Matthew's project finalized
ID_table_t VOLT_ID = {.CAN_ID = 0x100, .datatype = "v", .next = NULL};
ID_table_t TEMP_ID = {.CAN_ID = 0x200, .datatype = "t", .next = NULL};
ID_table_t SOC_ID = {.CAN_ID = 0x300, .datatype = "c", .next = NULL};
ID_table_t MOD_STATUS_ID = {.CAN_ID = 0x400, .datatype = "s", .next = NULL};
ID_table_t PACK_STATUS_ID = {.CAN_ID = 0x500, .datatype = "y", .next = NULL};

CAN_RxHeaderTypeDef RxHeader;

#define NEWLINE_CHAR "\n"
#define CAN_BYTES_SENT 8
#define TIMEOUT 100
#define TABLE_SIZE 5

#endif
