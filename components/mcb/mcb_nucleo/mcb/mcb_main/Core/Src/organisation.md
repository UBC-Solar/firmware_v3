# Organisation

## adc.c

## attributes.c

## can.c

## dma.c

## encoder.c (likely to be removed soon)

Contains the functions used to initialise and read the pedal's quadrature encoder using a hardware timer

This code has been written in bare-metal C and will likely soon be replaced with HAL function calls.

## freertos.c

This file contains all of the code that controls the behaviour of the MCB (minus interrupt service routines) _after_ the RTOS kernel starts running.

Essentially, all the important things that the MCB does (sending CAN messages, reading inputs, etc.) is possible because of the functions defined in this file.

If you are looking to add a new feature to the MCB, chances are you'll be creating a task in this file.

### Contents

- Thread handles
- RTOS initialisation function which creates OS objects (threads, queues, mutexes, etc.)
- Thread function prototypes and definitions

## gpio.c

## main.c

## stm32f1xx\_hal\_msp.c

## stm32f1xx\_hal\_timebase\_tim.c

## stm32f1xx\_it.c

## Other miscellaneous files
