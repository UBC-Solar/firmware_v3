#include "main.h"
#include "can.h"
#include "MDI_Helper_Functions.h"
#include "stm32f1xx_hal_can.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx.h"
#include <stdio.h>
#include <stdlib.h>


#define WHEEL_RADIUS 0.283 //in meters
#define MIN_VELOCITY 1.4  //lowest speed to activate cruise control in m/s, 5 km/h
#define CAR_MASS 250 //mass of car in kg
#define NOMINAL_PWR_OUT 2000//nominal power output from motor, in Watts
#define DRAG_COEFF 0.081003125


void PI_Controller_Outputs_wrapper(float setpoint_in, float velocity_in, float *output, float *nominal_force, float *uncapped_acceleration);
