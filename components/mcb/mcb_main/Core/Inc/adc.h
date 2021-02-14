#include "stm32f1xx_hal.h"

#ifndef ADC_MODULE
#define ADC_MODULE

#define ADC_ZERO_THRESHOLD 0x300

/**
 * Initializes an ADC for the regenerative braking toggle
 */
void ADCInit(void);

/**
 * Reads a value from the ADC
 */
uint16_t ReadADC(void);

#endif
