/*
 * diagnostic.c
 *
 *  Created on: Mar 27, 2025
 *      Author: Evan Owens
 */

/*	INCLUDES	*/
#include "diagnostic.h"
#include "CAN_comms.h"
#include "can.h"

/*	GLOBAL VARIABLES	*/
volatile uint32_t g_time_since_bootup = 0;
DRD_diagnostic_t g_diagnostics = {0};

/**
  * @brief  Sends the time since bootup via CAN
  * @retval None
*/
void DRD_time_since_bootup()
{
    CAN_comms_Tx_msg_t time_since_bootup_can_tx = {
        .data[0] = g_time_since_bootup & 0x000000FFU,
        .data[1] = g_time_since_bootup & 0x0000FF00U,
        .data[2] = g_time_since_bootup & 0x00FF0000U,
        .data[3] = g_time_since_bootup & 0xFF000000U,
        .header = time_since_bootup_can_header,
    };

    CAN_comms_Add_Tx_message(&time_since_bootup_can_tx);

}


/*	@brief Transmits DRD Diagnostic Messages over CAN
 *
 */
void DRD_diagnostics_transmit(DRD_diagnostic_t* diagnostics, bool from_ISR)
{
	CAN_comms_Tx_msg_t msg;
	msg.header = drd_diagnostic_header;

	msg.data[0] = (diagnostics->raw_adc1 & 0xFF);
	msg.data[1] = (diagnostics->raw_adc1 >> 8);
	msg.data[2] = (diagnostics->raw_adc2 & 0xFF);
	msg.data[3] = (diagnostics->raw_adc2 >> 8);
	msg.data[4] =  diagnostics->flags.all_flags;

	if (from_ISR)
	{
		CAN_comms_Add_Tx_messageISR(&msg);
	}
	else
	{
		CAN_comms_Add_Tx_message(&msg);
	}
}

