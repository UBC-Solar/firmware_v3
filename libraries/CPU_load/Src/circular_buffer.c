/*
 * circular_buffer.c
 *
 *  Created on: Nov 9, 2024
 *      Author: diego
 */

#include "circular_buffer.h"

bool circ_buf_full(circ_buf_t *p_circ_bbuf_t, uint8_t maxlen) {
    return (p_circ_bbuf_t->num_entries == maxlen);
}

bool CIRC_BUF_empty(circ_buf_t *p_circ_bbuf_t) {
    return (p_circ_bbuf_t->num_entries == 0);
}

int CIRC_BUF_enqueue(circ_buf_t *p_circ_buf_t, float data, uint8_t maxlen) {
    if (circ_buf_full(p_circ_buf_t, maxlen)) {
        // Buffer is full; overwrite the oldest entry
    	p_circ_buf_t->tail = (p_circ_buf_t->tail + 1) % maxlen;
    } else {
    	p_circ_buf_t->num_entries++;
    }

    // Insert the data and move the head
    p_circ_buf_t->pBuffer[p_circ_buf_t->head] = data;
    p_circ_buf_t->head = (p_circ_buf_t->head + 1) % maxlen;

    return 0;  // Success
}

