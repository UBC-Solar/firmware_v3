/*
 * BLINKY_blinky.c
 *
 *  Created on: Feb 2, 2022
 *      Author: Edward Ma
 *
 *      Purpose: Just to have Blinky code prepared for whenever
 *               we setup a new project and need test code.
 */

#include <BLINKY_bluepill.h>

/**
 *  This is to test code on the blue pill, which has a green LED on pin PC13
 *  Input:
 *      uint32_t time1 - time of light on in milliseconds
 *      uint32_t time2 - time in light off in milliseconds
 */
void BLINKY_bluepill(uint32_t time1, uint32_t time2)
{
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
    HAL_Delay(time1); //different delays to test which of GPIO_PIN_RESET and SET turns on or off the LED.
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
    HAL_Delay(time2);
}



