
/**
 *  @file heartbeat.c
 *  @brief Defines functions to track board's heartbeat using a global counter
 *
 *  @date 2024/13/06
 *  @author Aarjav Jain
 */


#include "heartbeat.h"

#ifndef RADIO_H_EXISTS                  // Check if radio.h exists to use the RADIO_tx_CAN_msg function
#if defined(__has_include) && __has_include("radio.h")
#include "radio.h"
#define RADIO_H_EXISTS
#endif
#endif

#ifndef RTC_H_EXISTS                    // Check if rtc.h exists to use the get_current_timestamp function
#if defined(__has_include) && __has_include("rtc.h")
#include "rtc.h"
#define RTC_H_EXISTS
#endif
#endif

#ifndef CAN_H_EXISTS                    // Check if can.h exists to use the CAN_Radio_msg_t struct
#if defined(__has_include) && __has_include("can.h")
#include "can.h"
#define CAN_H_EXISTS
#endif
#endif


uint16_t heartbeat_counter = 0;             // Global counter for sequential seconds since board start


/**
 * @brief heartbeat counter incrementer
 * @return void
 */
static void increment_hearbeat(void) {
    heartbeat_counter++;
}


/**
 * @brief Function to set the heartbeat CAN header
 * @param heartbeat_CAN_msg_header The CAN header to set
 * @param can_stdid The standard ID of the CAN message
 * @return void
 */
void set_heartbeat_CAN_header(CAN_TxHeaderTypeDef* heartbeat_CAN_msg_header, uint32_t can_stdid) {
    heartbeat_CAN_msg_header->StdId = can_stdid;
    heartbeat_CAN_msg_header->ExtId = CAN_NO_EXTID;
    heartbeat_CAN_msg_header->IDE = CAN_ID_STD;
    heartbeat_CAN_msg_header->RTR = CAN_RTR_DATA;
    heartbeat_CAN_msg_header->DLC = CAN_DATA_LENGTH;
}


/**
 * @brief Function to transmit the heartbeat CAN message if the board is TEL
 * @param heartbeat_CAN_msg_header The CAN header to transmit
 * @param can_stdid The standard ID of the CAN message
 * @param full_diagnostic_data The data array to send over CAN
 * @return void
 */
static void radio_transmit_if_TEL(CAN_TxHeaderTypeDef* heartbeat_CAN_msg_header, uint32_t can_stdid, uint8_t* full_diagnostic_data) {
    if (can_stdid == TEL_HEARTBEAT_STDID) {                                                      // If board is TEL then we will radio transmit
        #ifdef RTC_H_EXISTS
        #ifdef CAN_H_EXISTS
        #ifdef RADIO_H_EXISTS
            CAN_Radio_msg_t heartbeat_radio_msg;
            heartbeat_radio_msg.header = *heartbeat_CAN_msg_header;
            copy_to_buffer(heartbeat_radio_msg.data, DATA_START_INDEX, DATA_END_INDEX, full_diagnostic_data);
            heartbeat_radio_msg.timestamp.double_value = get_current_timestamp();

            RADIO_tx_CAN_msg(&heartbeat_radio_msg);
        #endif
        #endif
        #endif
    }
}


/**
 * @brief Function to send the heartbeat CAN message
 * @param can_stdid The standard ID of the CAN message
 * @param full_diagnostic_data The data array to send over CAN
 * @param hcan The CAN handle
 * @param can_mailbox The CAN mailbox
 * @return void
 */
static void send_heartbeat_CAN(uint32_t can_stdid, uint8_t* full_diagnostic_data, CAN_HandleTypeDef* hcan, uint32_t can_mailbox) {
    CAN_TxHeaderTypeDef heartbeat_CAN_msg_header;
    set_heartbeat_CAN_header(&heartbeat_CAN_msg_header, can_stdid);                              // Create CAN format header

    HAL_CAN_AddTxMessage(hcan, &heartbeat_CAN_msg_header, full_diagnostic_data, can_mailbox);    // Transmit heartbeat on CAN bus
    radio_transmit_if_TEL(&heartbeat_CAN_msg_header, can_stdid, full_diagnostic_data);           // Transmit heartbeat on radio if board is TEL
}


/**
 * @brief Function to pack the heartbeat counter into the 16 bits of MSB of the data array
 * @param data The data array to pack the heartbeat counter into
 * @return void
 */
static void pack_heartbeat_counter_msb(uint8_t* full_diagnostic_data) {
    full_diagnostic_data[COUNTER_BYTE_LO] = heartbeat_counter & BYTE_MASK;          // Set counter bytes starting MSB
    full_diagnostic_data[COUNTER_BYTE_HI] = (heartbeat_counter >> BITS_IN_BYTE) & BYTE_MASK;
}


/**
 * @brief Function to pack the user diagnostic data into the data array
 * @param full_diagnostic_data The data array to pack the user diagnostic data into
 * @param user_diagnostic_data The user diagnostic data to pack
 * @return void
 */
static void pack_diagnostic_data(uint8_t* full_diagnostic_data, uint8_t* user_diagnostic_data) {
    for (uint8_t i = 0; i < USER_DIAGNOSTIC_MSG_LENGTH; i++) {
        full_diagnostic_data[i] = user_diagnostic_data[i];
    }
}


/**
 * @brief Function to pack the heartbeat counter and user diagnostic data into the data array
 * @param full_diagnostic_data The data array to pack the heartbeat counter and user diagnostic data into
 * @param user_diagnostic_data The user diagnostic data to pack
 * @return void
 */
static void pack_data(uint8_t* full_diagnostic_data, uint8_t* user_diagnostic_data) {
    pack_heartbeat_counter_msb(full_diagnostic_data);                               // Put heartbeat counter in 16 bits of MSB
    pack_diagnostic_data(full_diagnostic_data, user_diagnostic_data);               // Put user diagnostic data in the rest of the array
}
    

/**
 * @brief Sends a heartbeat message over CAN using user's diagnostic data and updates runtime counter
 * @param can_stdid The standard ID of the CAN message. See heartbeat.h for your board's STDID
 * @param user_diagnostic_data The diagnostic data to send. 6 byte uint8_t array
 * @param hcan The CAN handle   
 * @param can_mailbox The CAN mailbox
 * @return void
 */
void HEARTBEAT_handler(uint32_t can_stdid, uint8_t* user_diagnostic_data, CAN_HandleTypeDef* hcan, uint32_t can_mailbox) {
    uint8_t full_diagnostic_data[CAN_DATA_LENGTH];                                  // Data array to send over CAN

    pack_data(full_diagnostic_data, user_diagnostic_data);                          // Pack the data array with user diagnostic data and heartbeat counter
    send_heartbeat_CAN(can_stdid, full_diagnostic_data, hcan, can_mailbox);         // Send the heartbeat msg over CAN
    increment_hearbeat();                                                           // Increment since a second has passed
}
