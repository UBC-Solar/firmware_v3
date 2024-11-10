/*
 * circular_buffer.h
 *
 *  Created on: Nov 9, 2024
 *      Author: diego
 */

#ifndef CPU_LOAD_INC_CIRCULAR_BUFFER_H_
#define CPU_LOAD_INC_CIRCULAR_BUFFER_H_

#define MAX_CPU_LOAD_ENTRIES 20

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    float *pBuffer;
    int head, tail, num_entries;
} circ_buf_t;

int CIRC_BUF_enqueue(circ_buf_t *p_circ_buf_t, float data, uint8_t maxlen);
bool CIRC_BUF_empty(circ_buf_t *p_circ_buf_t);

#endif /* CPU_LOAD_INC_CIRCULAR_BUFFER_H_ */
