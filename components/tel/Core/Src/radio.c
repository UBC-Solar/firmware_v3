
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
#include "tel_freertos.h"


/* PRIVATE DEFINES */
#define NO_PRIORITY                                 0
#define NON_BLOCKING                                0
#define ID_DELIMITER_CHAR                           '#'
#define CARRIAGE_RETURN_CHAR                        '\r'
#define NEW_LINE_CHAR                               '\n'
#define MASK_4_BITS                                 0xF


/* PRIVATE FUNCTIONS DECLARATIONS */
void set_radio_msg(CAN_RxHeaderTypeDef* header, uint8_t* data, RADIO_Msg_TypeDef* radio_msg);
uint64_t get_timestamp();
uint32_t get_can_id(CAN_RxHeaderTypeDef* can_msg_header_ptr);
uint8_t get_data_length(uint32_t DLC);


/**
 * @brief Adds a radio message to the radio tx queue
 * 
 * @param CAN_comms_Rx_msg Pointer to the CAN Rx message
 */
void RADIO_filter_and_queue_msg(CAN_comms_Rx_msg_t* CAN_comms_Rx_msg)
{
	// TODO: Implement filtering
	// EX: if (check_CAN_ID_whitelist(CAN_comms_Rx_msg->header.StdId) == true) { ... }

	/* Create radio message struct */
	RADIO_Msg_TypeDef radio_msg = {0};
	set_radio_msg(&(CAN_comms_Rx_msg->header), CAN_comms_Rx_msg->data, &radio_msg);

	/* Add CAN message to radio tx queue */
	osMessageQueuePut(radio_tx_queue, &radio_msg, NO_PRIORITY, NON_BLOCKING);
}


/**
 * @brief Radio Tx task that sends radio messages over UART
 * This tasks waits for a message in the radio tx queue and acquires a USART Tx semaphore before sending the message over UART
 * 
 */
void RADIO_Tx_forever()
{
	/* Infinite Loop */
	for(;;)
	{
		/* Wait until there is a message in the queue */ 
		RADIO_Msg_TypeDef radio_msg;
        if (HAL_GPIO_ReadPin(RADIO_CTS_GPIO_Port, RADIO_CTS_Pin) == GPIO_PIN_RESET)
        {
            if (osOK == osMessageQueueGet(radio_tx_queue, &radio_msg, NULL, osWaitForever))
            {
                UART_radio_transmit(&radio_msg);
            }
            else
            {
                // TODO: Error handle
            }
        }
	}
}


/**
 * @brief Sets all the fields in the radio message struct
 * 
 * @param header The CAN header struct
 * @param data The CAN data
 */
void set_radio_msg(CAN_RxHeaderTypeDef* header, uint8_t* data, RADIO_Msg_TypeDef* radio_msg)
{
	memset(radio_msg, 0, sizeof(RADIO_Msg_TypeDef));           // 0 out all 8 bytes data
	
	radio_msg->timestamp        = get_timestamp();
	radio_msg->can_id           = get_can_id(header);
	radio_msg->ID_DELIMETER     = ID_DELIMITER_CHAR;
	memcpy(radio_msg->data, data, RADIO_DATA_LENGTH);
	radio_msg->data_len         = get_data_length(header->DLC);
	radio_msg->CARRIAGE_RETURN  = CARRIAGE_RETURN_CHAR;
	radio_msg->NEW_LINE         = NEW_LINE_CHAR;
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
uint8_t get_data_length(uint32_t DLC)
{
	return (uint8_t) (DLC & MASK_4_BITS);
}