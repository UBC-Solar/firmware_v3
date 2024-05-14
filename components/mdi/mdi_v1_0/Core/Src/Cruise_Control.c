#include "main.h"
#include "Cruise_Control.h"
#include "stm32f1xx_hal_can.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx.h"
#include <stdio.h>
#include <stdlib.h>

/*void PI_Controller_Outputs_wrapper(float setpoint_in,
                       	   	   	   float velocity_in,
								   double *output,
								   double *nominal_force,
								   double *uncapped_acceleration)
{

	// Calculate PI controller output
	double f_nominal, a_nominal, f_loss;

	//calculates the most efficient acceleration
	a_nominal = 7.52 / velocity_in;
	//multiplies by mass to get the most efficient force, f_nominal
	f_nominal = a_nominal * CAR_MASS;

	//Finds the power loss through tire friction and the air resistance, and sets that as a lower bound to out, or maintains speed
	f_loss = 17.15 + (DRAG_COEFF * velocity_in * velocity_in);

	// Constants for the PI controller
	double Ki = 0.0005 * CAR_MASS; // Integral gain
	double Kp = 0.5 * CAR_MASS;   //proportional gain
	// Static variables to maintain integral state
	static double integral = 0.0;

	// Calculate the error
	double error = setpoint_in - velocity_in;

	// Calculate the integral term (accumulating the error over time)
	integral += error;

	// Calculate the control output (throttle)
	double controlOutput = Kp * error + Ki * integral;
	double u_out = controlOutput / 250;


	if (velocity_in > MIN_VELOCITY){ //only allows the velocity to be set nominally above a minimum speed
		if (controlOutput > f_nominal) { //prevents acceleration from exceeding nominal value
			controlOutput = f_nominal;
		}
	}
	// Set the output to the control output
	*output = controlOutput;
	*nominal_force = f_nominal;
	*uncapped_acceleration = u_out;

 return;
}*/
