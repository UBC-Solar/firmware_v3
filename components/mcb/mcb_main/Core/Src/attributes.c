/*
 * attributes.c
 *
 *  Created on: Feb. 23, 2021
 *      Author: Mihir Nimgade
 */

#include "attributes.h"

// <----- RTOS object attributes ----->

const osThreadAttr_t readEncoderTask_attributes = {
        .name = "readEncoder",
        .priority = (osPriority_t) osPriorityHigh,
        .stack_size = 128 * 4
};

const osThreadAttr_t sendMotorCommandTask_attributes = {
        .name = "sendMotorCommand",
        .priority = (osPriority_t) osPriorityHigh,
        .stack_size = 128 * 4
};

const osThreadAttr_t readRegenValueTask_attributes = {
        .name = "readRegenValue",
        .priority = (osPriority_t) osPriorityHigh,
        .stack_size = 128 * 4
};

const osThreadAttr_t sendCruiseCommandTask_attributes = {
        .name = "sendCruiseCommandTask",
        .priority = (osPriority_t) osPriorityHigh,
        .stack_size = 128 * 4
};

const osThreadAttr_t updateEventFlagsTask_attributes = {
        .name = "updateEventFlags",
        .priority = (osPriority_t) osPriorityHigh,
        .stack_size = 128 * 4
};

const osThreadAttr_t sendRegenCommandTask_attributes = {
        .name = "sendRegenCommand",
        .priority = (osPriority_t) osPriorityHigh,
        .stack_size = 128 * 4
};

const osTimerAttr_t encoderTimer_attributes = {
        .name = "encoderTimer",
        .attr_bits = 0,
        .cb_mem = NULL,
        .cb_size = 0
};

const osMessageQueueAttr_t encoderQueue_attributes = {
        .name = "encoderQueue"
};
