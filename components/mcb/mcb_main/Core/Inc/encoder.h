#include "stm32f1xx_hal.h"

#ifndef ENCODER_MODULE
#define ENCODER_MODULE

#define PEDAL_MAX 0xD0
#define PEDAL_MIN 0x0F
#define PEDAL_OVERLOAD 0xFF

/**
 * Initializes the encoder for the accelerator pedal
 */
void EncoderInit(void);

/**
 * Reads a value from the encoder
 */
uint16_t EncoderRead(void);

#endif
