#include "encoder.h"
#include "stm32f1xx_hal.h"

/**
 * Initializes the encoder for the accelerator pedal.
 */
void EncoderInit(void) {

    // the APB2ENR enables clocks for the peripherals

    //Initialize AFIO Clock
    RCC->APB2ENR |= 0x1;

    //TIM 1 Enable
    RCC->APB2ENR |= 0x1 << 11;

    //Initialize GPIOA Clock
    RCC->APB2ENR |= 0x1 << 2;
    GPIOA->CRH &= 0;

    // configures PA8 as floating input
    // also configures PA9 as floating input
    GPIOA->CRH |= 0x00000044;

    //TIM 1 No Remap
    AFIO->MAPR &= ~(0x11 << 6);

    TIM1->CCMR1 |= 0x01;    //Map Channel 1 to TI1
    TIM1->CCMR1 |= 0x01 << 8; //Map Channel 2 to TI2

    TIM1->CCER &= ~(0x1 << 1);
    TIM1->CCER &= ~(0x1 << 3);

    TIM1->CCER &= ~(0x1 << 5);
    TIM1->CCER &= ~(0x1 << 7);

    TIM1->CCER |= 0x1;
    TIM1->CCER |= 0x1 << 4;

    TIM1->SMCR |= 0x011;

    TIM1->CR1 |= 0x1;

}

/**
 * Reads a value from the encoder.
 */
uint16_t EncoderRead(void) {

    uint16_t Encoder_Reading = TIM1->CNT;

    //TEST
    //SendString("    Raw Encoder Reading:");
    //SendInt(Encoder_Reading);
    //SendLine();
    //ENDTEST

    if (Encoder_Reading < PEDAL_MIN) {
        Encoder_Reading = 0;
    } else if (Encoder_Reading < PEDAL_MAX) {
        Encoder_Reading = Encoder_Reading - PEDAL_MIN;

        if (Encoder_Reading > 0x40) {
            Encoder_Reading = 0x10;
        } else {
            Encoder_Reading = 0x00;
        }

    } else if (Encoder_Reading < PEDAL_OVERLOAD) {
        Encoder_Reading = 0x10;
    } else {
        Encoder_Reading = 0;
    }

    return Encoder_Reading;

}
