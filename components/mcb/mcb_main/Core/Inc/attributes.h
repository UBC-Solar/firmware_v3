/*
 * attributes.h
 *
 *  Created on: Feb. 23, 2021
 *      Author: Mihir Nimgade
 */

#ifndef INC_ATTRIBUTES_H_
#define INC_ATTRIBUTES_H_

#include "cmsis_os.h"

extern const osThreadAttr_t readEncoderTask_attributes;
extern const osThreadAttr_t sendMotorCommandTask_attributes;
extern const osThreadAttr_t readRegenValueTask_attributes;
extern const osThreadAttr_t updateEventFlagsTask_attributes;
extern const osThreadAttr_t updateEventFlagsTask_attributes;

extern const osTimerAttr_t encoderTimer_attributes;

extern const osMessageQueueAttr_t encoderQueue_attributes;

#endif /* INC_ATTRIBUTES_H_ */
