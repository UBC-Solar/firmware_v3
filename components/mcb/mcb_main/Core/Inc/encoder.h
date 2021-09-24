#include "stm32f1xx_hal.h"

#ifndef ENCODER_MODULE
#define ENCODER_MODULE

/** Stores maximum pedal value */
#define PEDAL_MAX 		0xFF

/** Stores minimum pedal value */
#define PEDAL_MIN 		0x00

#define PEDAL_OVERLOAD 	0xFF

/**
 * Initializes the encoder for the accelerator pedal
 */
void EncoderInit(void);

/**
 * Reads a value from the encoder
 */
uint16_t EncoderRead(void);

#endif
