
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
#include "rtc.h"


/* LOCAL GLOBALS (SG) */
static volatile uint8_t sg_rx_queue_index = 0;								// Tracker for new messages. Only set in CAN Rx callback
static volatile uint8_t sg_tx_queue_index = 0;								// Tracker for messages to be sent over radio. Not always the same as sg_rx_queue_index
static RADIO_Msg_TypeDef template_radio_msg = {0};						    // Template for every CAN message
static RADIO_QueueMsg_TypeDef sg_rx_queue[MAX_RX_QUEUE_SIZE] = {0};		


/**
 * @brief Initializes the RX queue elements to need sending. Also initializes the template radio message
 */
void RADIO_init()
{
	// Queue Init
	for (uint8_t i = 0; i < MAX_RX_QUEUE_SIZE; ++i)
	{
		sg_rx_queue[i].needs_sending = true;
	}

	// Radio Msg Init
	memset(&template_radio_msg, 0, sizeof(RADIO_Msg_TypeDef));

	template_radio_msg.ID_DELIMETER = ID_DELIMITER_CHAR;
	template_radio_msg.CARRIAGE_RETURN = CARRIAGE_RETURN_CHAR;
	template_radio_msg.NEW_LINE = NEW_LINE_CHAR;
}


/**
 * @brief Checks if the message needs to be sent over radio
 */
bool RADIO_needs_sending(RADIO_QueueMsg_TypeDef* queue_msg)
{
	return queue_msg->needs_sending;
}


static uint32_t can_msg_counter = 0;
/**
 * @brief Sends the queue message via UART to XBee radio module for OTA Tx
 */
void RADIO_send_msg_uart()
{
	RADIO_QueueMsg_TypeDef* current_queue_msg_ptr = &sg_rx_queue[sg_tx_queue_index]; // Get the current queue msg
	uint8_t needs_sending = (uint8_t)(current_queue_msg_ptr->needs_sending);		 // Save because of reuse

	switch (needs_sending)
	{
		case MSG_READY_TO_SEND:
			double timestamp = RTC_get_timestamp_secs();                      // Get the timestamp immediately ASAP
			can_msg_counter += (uint32_t)needs_sending;

			set_radio_msg(current_queue_msg_ptr, timestamp);			      // Standardize to match sunlink
			// UART_radio_transmit(&(current_queue_msg_ptr->radio_msg));	  // Sending to Module via DMA

			current_queue_msg_ptr->needs_sending = false;
			increment_to_next_tx_msg();									      // Move to next message to tx

			break;

		case MSG_NOT_READY_TO_SEND:
			// TODO Error catching, diagnositc message, etc.
			break;
		
		default:
			break;
	}
}


/**
 * @brief Getter for the current message in the RX queue
 * 
 * @return The current message in the RX queue
 */
RADIO_QueueMsg_TypeDef* RADIO_get_rx_msg()
{
	return &(sg_rx_queue[sg_rx_queue_index]);
}


/**
 * @brief Increments the index of the rx queue tracker.
 * 
 * Postcondition: <code>sg_tx_queue_index = sg_rx_queue_index</code> and <code>sg_rx_queue_index</code> is incremented
 * 
 * This is done after the message has been received in the CAN Rx callback
 */
void RADIO_increment_rx_queue_index()
{
	sg_tx_queue_index = sg_rx_queue_index;	
	sg_rx_queue_index = CIRCULAR_INCREMENT_SET(sg_rx_queue_index, MAX_RX_QUEUE_SIZE);
}


/**
 * @brief Sets the timestamp in the radio message as a uint64_t
 * 
 * @param timestamp The timestamp to set in seconds. 
 */
void set_timestamp(double timestamp)
{
	DoubleAsUint64 timestamp_union;
	timestamp_union.d = timestamp;
	template_radio_msg.timestamp = BITOPS_64BIT_REVERSE(timestamp_union.u);	 // Flip for sunlink
}


/**
 * @brief Getter for CAN ID inside the CAN header struct
 * 
 * @param can_msg_header_ptr The CAN header struct
 * 
 * @return The CAN ID as a 32-bit unsigned integer to account for both standard and extended IDs
 */
uint32_t get_can_id(CAN_RxHeaderTypeDef* can_msg_header_ptr)
{
	return (can_msg_header_ptr->IDE == CAN_ID_STD) ? can_msg_header_ptr->StdId : can_msg_header_ptr->ExtId;
}


/**
 * @brief Sets CAN ID in the radio message as a uint32_t
 * 
 * @param can_msg_header_ptr The CAN header struct with CAN ID
 */
void set_can_id(CAN_RxHeaderTypeDef* can_msg_header_ptr)
{
	uint32_t can_id = get_can_id(can_msg_header_ptr);
	template_radio_msg.can_id = BITOPS_32BIT_REVERSE(can_id);				 // Flip ID for sunlink
}


/**
 * @brief Sets the CAN data in the radio message 
 * 
 * @param queue_msg_ptr The queue message containing CAN data
 */
void set_can_data(RADIO_QueueMsg_TypeDef* queue_msg_ptr)
{
	memcpy(&(template_radio_msg.data[START_OF_ARRAY]), &(queue_msg_ptr->data[START_OF_ARRAY]), RADIO_DATA_LENGTH);
}


/**
 * @brief Sets the data length in the radio message
 * 
 * @param DLC The data length code from the CAN header
 */
void set_data_length(uint8_t DLC)
{
	template_radio_msg.data_len = DLC & MASK_4_BITS;
}


/**
 * @brief Prepares the radio message before sending it over UART
 * 
 * @param queue_msg The queue message containing CAN data and header
 */
void set_radio_msg(RADIO_QueueMsg_TypeDef* queue_msg_ptr, double timestamp)
{
	CAN_RxHeaderTypeDef* can_msg_header_ptr = &(queue_msg_ptr->header);

	set_timestamp(timestamp);
	set_can_id(can_msg_header_ptr);
	set_can_data(queue_msg_ptr);
	set_data_length(can_msg_header_ptr->DLC);
}


/**
 * @brief Increments the index of the tx queue tracker.
 */
void increment_to_next_tx_msg()
{
	sg_tx_queue_index = CIRCULAR_INCREMENT_SET(sg_tx_queue_index, MAX_RX_QUEUE_SIZE);
}
