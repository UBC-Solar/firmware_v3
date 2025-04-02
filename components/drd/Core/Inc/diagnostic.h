/*
 * diagnostic.h
 *
 *  Created on: Mar 27, 2025
 *      Author: Evan Owens
 */

#ifndef INC_DIAGNOSTIC_H_
#define INC_DIAGNOSTIC_H_

/* INCLUDES */
#include <stdbool.h>
#include <stdint.h>

/*	DATA TYPES 	*/
typedef union {
	struct {
		volatile bool mech_brake_pressed 		: 1;
		volatile bool regen_enabled 			: 1;
		volatile bool throttle_ADC_out_of_range : 1;
		volatile bool throttle_ADC_mismatch 	: 1;
		volatile bool watchdog_reset 			: 1;
		volatile bool motor_comm_fault 			: 1;
	};
	uint8_t all_flags;
} DRD_flags_t;

typedef struct {
	volatile uint16_t raw_adc1;
	volatile uint16_t raw_adc2;
	DRD_flags_t flags;
} DRD_diagnostic_t;



/*	GLOBAL VARIABLES	*/
extern volatile uint32_t g_time_since_bootup;
extern DRD_diagnostic_t g_diagnostics;


/*	Function Prototypes	*/
void DRD_time_since_bootup();
void DRD_diagnostics_transmit(DRD_diagnostic_t* diagnostics, bool from_ISR);



#endif /* INC_DIAGNOSTIC_H_ */
