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
        .priority = (osPriority_t) osPriorityNormal,
        .stack_size = 128 * 4
};

const osThreadAttr_t sendRegenCommandTask_attributes = {
        .name = "sendRegenCommand",
        .priority = (osPriority_t) osPriorityHigh,
        .stack_size = 128 * 4
};

const osThreadAttr_t sendCruiseCommandTask_attributes = {
        .name = "sendCruiseCommandTask",
        .priority = (osPriority_t) osPriorityBelowNormal,
        .stack_size = 128 * 4
};

const osThreadAttr_t sendIdleCommandTask_attributes = {
        .name = "sendIdleCommandTask",
        .priority = (osPriority_t) osPriorityNormal,
        .stack_size = 128 * 4
};

const osThreadAttr_t updateEventFlagsTask_attributes = {
        .name = "updateEventFlags",
        .priority = (osPriority_t) osPriorityHigh,
        .stack_size = 128 * 4
};

const osThreadAttr_t receiveBatteryMessageTask_attributes = {
        .name = "receiveBatteryMessage",
        .priority = (osPriority_t) osPriorityLow,
        .stack_size = 128 * 4
};

const osMessageQueueAttr_t encoderQueue_attributes = {
        .name = "encoderQueue"
};
