#include "ADC.h"

// 0 - 300 hex pot

/**
 * Initializes an ADC for the regenerative braking toggle
 */
void ADCInit(void){

	// Init PA0 to analog input (0x0)
	GPIOA->CRL &= ~(0xFUL);

	// Configure ADC2
	// Enable ADC clock
	RCC->APB2ENR |= RCC_APB2ENR_ADC2EN;

	// Change control registers
	// By default, CR1 is set correctly
	ADC2->CR2 |= ADC_CR2_CONT;

	// Change the sample time register 2
	// sets a sample time of 239.5 cycles for channel 0 of the ADCs
	ADC2->SMPR2 |= 0x7UL;

	// Start ADC
	ADC2->CR2 |= ADC_CR2_ADON;
	ADC2->CR2    |= 0x8UL;
	while ((ADC2->CR2 >> 3) & 0x1UL);
	ADC2->CR2    |= 0x4UL;
	while ((ADC2->CR2 >> 2) & 0x1UL);
	ADC2->CR2 |= 0x1UL;

}

/**
 * Reads a value from the ADC
 */
uint16_t ReadADC(void){

	uint16_t ADC_reading = ADC2->DR;

	//SendString("    Raw ADC Reading:");
	//SendInt(ADC_reading);

	/*
	if (ADC_reading > ADC_ZERO_THRESHOLD)
	{
		ADC_reading = 0x000;
	}
	else
	{
		ADC_reading = ADC_ZERO_THRESHOLD - ADC_reading;
	}
	*/

	return ADC_reading;
}
