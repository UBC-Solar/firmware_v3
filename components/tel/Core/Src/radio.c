
/**
 *  @file radio.c
 *  @brief Defines functions packaging data into a CAN buffer and sending it over UART
 *
 *  @date 2024/10/10
 *  @author Aarjav Jain
 */


#include "radio.h"


/* INCLUDES */
#include "bitops.h"
#include "usart.h"
#include "string.h"


/* LOCAL GLOBALS (SG) */
static volatile uint8_t sg_rx_queue_index = 0;								// Tracker for new messages. Only set in CAN Rx callback
static volatile uint8_t sg_tx_queue_index = 0;								// Tracker for messages to be sent over radio. Not always the same as sg_rx_queue_index
static RADIO_QueueMsg_TypeDef sg_rx_queue[MAX_RX_QUEUE_SIZE] = {0};		


/**
 * @brief Prepares the radio message for transmission by changing various fields
 * 
 * These changes are mode post CAN Rx callback to reduce callback time
 * 
 * @param radio_msg The message to prepare of type RADIO_Msg_TypeDef
 */
void prep_radio_msg(RADIO_Msg_TypeDef* radio_msg, double timestamp)
{
	DoubleAsUint64 timestamp_union;
	timestamp_union.d = timestamp;	 

	radio_msg->timestamp = BITOPS_64BIT_REVERSE(timestamp_union.u);    		// Need to flip for sunlink to parse correctly	                
}


/**
 * @brief Increments the index of the tx queue tracker.
 */
void increment_to_next_tx_msg()
{
	sg_tx_queue_index = CIRCULAR_INCREMENT_SET(sg_tx_queue_index, MAX_RX_QUEUE_SIZE);
}


/**
 * @brief Sends the queue message via UART to XBee radio module for OTA Tx
 */
void RADIO_send_msg_uart(double timestamp)
{
	RADIO_QueueMsg_TypeDef* current_queue_msg_ptr = &sg_rx_queue[sg_tx_queue_index];	// Get the current queue msg

	if (!(current_queue_msg_ptr->is_sent))		
	{
		RADIO_Msg_TypeDef* radio_msg = &(current_queue_msg_ptr->radio_msg);	// Get the radio message to send

		prep_radio_msg(radio_msg, timestamp);	                        	// Modify various fields for sunlink compatibility
		UART_radio_transmit(radio_msg);	                                    // Sending to Module via DMA

		current_queue_msg_ptr->is_sent = true;	     						// Mark as sent                       
	}

	increment_to_next_tx_msg();												// Move to next message to tx
}


/**
 * @brief Checks if the message has been sent over radio
 */
bool RADIO_is_msg_sent(RADIO_QueueMsg_TypeDef* queue_msg)
{
	return queue_msg->is_sent;
}


/**
 * @brief Initializes the RX queue elements with necessary constants
 *        that sunlink requires (the ID_DELIMETER) and printing requires (\r\n).
 */
void RADIO_queue_init()
{
	for (uint8_t i = 0; i < MAX_RX_QUEUE_SIZE; ++i)
	{
		sg_rx_queue[i].is_sent = true;

		sg_rx_queue[i].radio_msg.ID_DELIMETER      = ID_DELIMITER_CHAR;
		sg_rx_queue[i].radio_msg.CARRIAGE_RETURN   = CARRIAGE_RETURN_CHAR;
		sg_rx_queue[i].radio_msg.NEW_LINE          = NEW_LINE_CHAR;
	}
}


void RADIO_set_rx_msg(uint32_t can_id, uint8_t* can_data, uint32_t DLC)
{
	RADIO_QueueMsg_TypeDef* current_queue_message = &sg_rx_queue[sg_rx_queue_index];

	if ((current_queue_message->is_sent))		// If the message is not sent, do not overwrite
	{
		/* Set all radio message fields minimally */
		current_queue_message->radio_msg.can_id_reversed = BITOPS_32BIT_REVERSE(can_id);	// Flip for readability and sunlink
		memcpy(&(current_queue_message->radio_msg.data[START_OF_ARRAY]), &can_data[START_OF_ARRAY], RADIO_DATA_LENGTH);
		current_queue_message->radio_msg.data_len = DLC & MASK_4_BITS;

		current_queue_message->is_sent = false;									// 'Notify' to Tx function to send this message

		sg_tx_queue_index = sg_rx_queue_index;					     		    // Move Tx tracker to Rx Tracker for immediate Tx
		sg_rx_queue_index = CIRCULAR_INCREMENT_SET(sg_rx_queue_index, MAX_RX_QUEUE_SIZE);		// Move Rx tracker to find a free spot
	}
	else {}		// If not sent then ERROR. Radio Tx might be too slow
}
