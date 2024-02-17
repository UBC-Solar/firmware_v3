#ifndef __CYCLIC_DATA_H__
#define __CYCLIC_DATA_H__

#ifdef __cplusplus
    extern "C" {
#endif

#include <stdint.h>
#include <float.h>

typedef struct {
    int8_t  data_value;
    uint32_t last_set_time;
    uint32_t max_cycle_time;
} CyclicDataS8;
typedef struct {
    uint8_t  data_value;
    uint32_t last_set_time;
    uint32_t max_cycle_time;
} CyclicDataU8;

typedef struct {
    uint16_t data_value;
    uint32_t last_set_time;
    uint32_t max_cycle_time;
} CyclicDataU16;

typedef struct {
    uint32_t data_value;
    uint32_t last_set_time;
    uint32_t max_cycle_time;
} CyclicDataU32;

typedef struct {
    float    data_value;
    uint32_t last_set_time;
    uint32_t max_cycle_time;
} CyclicDataFloat;

// Creates cyclic data structs

// Signed 8 bit int
#define CREATE_CYCLIC_DATA_S8(name, max_cycle_time) \
    CyclicDataS8 name = {0, UINT32_MAX, max_cycle_time}

// Unsigned 8 bit int
#define CREATE_CYCLIC_DATA_U8(name, max_cycle_time) \
    CyclicDataU8 name = {0, UINT32_MAX, max_cycle_time}

// Unsigned 16 bit int
#define CREATE_CYCLIC_DATA_U16(name, max_cycle_time) \
    CyclicDataU16 name = {0, UINT32_MAX, max_cycle_time}

// Unsigned 32 bit int
#define CREATE_CYCLIC_DATA_U32(name, max_cycle_time) \
    CyclicDataU32 name = {0, UINT32_MAX, max_cycle_time}

// Float
#define CREATE_CYCLIC_DATA_FLOAT(name, max_cycle_time) \
    CyclicDataFloat name = {0, UINT32_MAX, max_cycle_time}

// Sets cyclic data and value and saves time it was set
#define SET_CYCLIC_DATA(name, new_value) \
    name.data_value = new_value; \
    name.last_set_time = HAL_GetTick(); \

// Returns a pointer to the cyclic data value. If the current time - last_set_time is greater than the max cycle time, then it returns NULL
#define GET_CYCLIC_DATA(name) ((name.max_cycle_time) > (HAL_GetTick() - name.last_set_time) ? &(name.data_value) : NULL)

#endif
    
