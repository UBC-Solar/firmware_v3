
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


/* LOCAL GLOBALS */
static RADIO_Msg_TypeDef template_radio_msg = {0};						    // Template for every CAN message


/* PRIVATE FUNCTIONS DECLARATIONS */
void set_template_radio_msg(CAN_RxHeaderTypeDef* header, uint8_t* data);
uint64_t get_timestamp();
uint32_t get_can_id(CAN_RxHeaderTypeDef* can_msg_header_ptr);
uint8_t get_data_length(uint8_t DLC);



/**
 * @brief Initializes the template radio message
 */
void RADIO_init()
{
	memset(&template_radio_msg, 0, sizeof(RADIO_Msg_TypeDef));		// Init template msg
	template_radio_msg.ID_DELIMETER = ID_DELIMITER_CHAR;
	template_radio_msg.CARRIAGE_RETURN = CARRIAGE_RETURN_CHAR;
	template_radio_msg.NEW_LINE = NEW_LINE_CHAR;
}


static uint32_t counter = 0;
/**
 * @brief Sends a CAN message over UART to the radio module
 * 
 * @param header The CAN header struct
 * @param data The CAN data
 */
void RADIO_send_msg_uart(CAN_RxHeaderTypeDef* header, uint8_t* data)
{
	set_template_radio_msg(header, data);
	UART_radio_transmit(&template_radio_msg);
	counter++;
}


/**
 * @brief Sets all the fields in the radio message struct
 * 
 * @param header The CAN header struct
 * @param data The CAN data
 */
void set_template_radio_msg(CAN_RxHeaderTypeDef* header, uint8_t* data)
{
	template_radio_msg.timestamp         = get_timestamp();
	template_radio_msg.can_id            = get_can_id(header);
	/* Get CAN Data */          		 memcpy(template_radio_msg.data, data, RADIO_DATA_LENGTH);
	template_radio_msg.data_len          = get_data_length(header->DLC);
}


/**
 * @brief Getter for the timestamp in the radio message as a 64-bit unsigned integer
 * 
 * @return The timestamp as a 64-bit unsigned integer
 */
uint64_t get_timestamp()
{
	DoubleAsUint64 timestamp_union;
	timestamp_union.d = RTC_get_timestamp_secs();
	return BITOPS_64BIT_REVERSE(timestamp_union.u);
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
	uint32_t can_id = (can_msg_header_ptr->IDE == CAN_ID_STD) ? can_msg_header_ptr->StdId : can_msg_header_ptr->ExtId;
	return BITOPS_32BIT_REVERSE(can_id);
}


/**
 * @brief Gets the data length in the radio message
 * 
 * @param DLC The data length code from the CAN header
 * 
 * @return The data length as an 8-bit unsigned integer but only the 4 least significant bits are used
 */
uint8_t get_data_length(uint8_t DLC)
{
	return DLC & MASK_4_BITS;
}
