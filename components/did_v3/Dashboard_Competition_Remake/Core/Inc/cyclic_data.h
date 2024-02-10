#ifndef __CYCLIC_DATA_H__
#define __CYCLIC_DATA_H__

#ifdef __cplusplus
    extern "C" {
#endif

#include <stdint.h>

typedef struct {
    uint32_t data_value;
    uint32_t last_set_time;
    uint32_t max_cycle_time;
} CyclicDataU32;


#define CREATE_CYCLIC_DATA_U32(name, max_cycle_time) \
    CyclicDataU32 name = {0, UINT32_MAX, max_cycle_time}

// Sets 
#define SET_CYCLIC_DATA_U32(name, new_value) \
    name.data_value = new_value; \
    name.last_set_time = HAL_GetTick(); \

// Returns a pointer to the cyclic data value. If the cycle time is greater than the max cycle time, then it returns NULL
#define GET_CYCLIC_DATA_U32(name) ((name.max_cycle_time) > (HAL_GetTick() - name.last_set_time) ? &(name.data_value) : NULL)

#endif
    
