
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
 #include "can.h"
 #include "gps.h"
 #include "imu.h"
 
 /* PRIVATE DEFINES */
 #define NO_PRIORITY                                 0
 #define NON_BLOCKING                                0
 #define ID_DELIMITER_CHAR                           '#'
 #define CARRIAGE_RETURN_CHAR                        '\r'
 #define NEW_LINE_CHAR                               '\n'
 #define MASK_4_BITS                                 0xF
 
 
 /* PRIVATE FUNCTIONS DECLARATIONS */
 void set_radio_msg(CAN_RxHeaderTypeDef* header, uint8_t* data, RADIO_Msg_TypeDef* radio_msg);
 void set_radio_msg_tx(CAN_TxHeaderTypeDef* header, uint8_t* data, RADIO_Msg_TypeDef* radio_msg);
 uint64_t get_timestamp();
 uint32_t get_can_id(CAN_RxHeaderTypeDef* can_msg_header_ptr);
 
 
 Radio_diagnostics_t Radio_diagnostic = {
 
     .dropped_radio_msg = 0,
     .radio_hal_transmit_failures = 0,
     .successful_radio_tx = 0
 };
 
 uint8_t get_data_length(uint32_t DLC);
 
 typedef struct {
     uint32_t id;
     uint8_t  mod;     // transmit on radio only every 'mod' occurrences
     uint32_t count;   // count of received messages
 } CanFilter_t;
 
 // Definitions for CAN IDs (add or adjust as needed)
 #define DRD_DIAGNOSTICS_ID                  0x403
 #define MDI_DIAGNOSTICS_ID                  0x501
 #define STR_DIAGNOSTICS_ID                  0x581
 #define TEL_DIAGNOSTICS_ID                  0x751
 
 #define DRD_TIME_SINCE_BOOTUP_ID            0x404
 #define MDI_TIME_SINCE_BOOTUP_ID            0x500
 #define STR_TIME_SINCE_BOOTUP_ID            0x582
 #define TEL_TIME_SINCE_BOOTUP_ID            0x750
 
 #define GPS_LONG_LAT_ID                     0x756
 
 #define DRD_MOTOR_COMMAND_ID                0x401
 
 #define ECU_STATUS_ID                       0x450
 #define BMS_FAULTS_ID                       0x622
 #define BMS_VOLTAGE_SUMMARY_VOLTAGE_ID      0x623
 #define BMS_PACK_HEALTH_ID                  0x624
 #define BMS_TEMP_SUMMARY_ID                 0x625
 #define BMS_MODULE_VOLTAGES_ID              0x626
 #define BMS_MODULE_TEMPERATURES_ID          0x627
 
 #define MPPT_A_INPUT_MEASUREMENTS_ID          0x6A0
 #define MPPT_B_INPUT_MEASUREMENTS_ID          0x6B0
 #define MPPT_C_INPUT_MEASUREMENTS_ID          0x6C0
 
 #define MPPT_A_OUTPUT_MEASUREMENTS_ID         0x6A1
 #define MPPT_B_OUTPUT_MEASUREMENTS_ID         0x6B1
 #define MPPT_C_OUTPUT_MEASUREMENTS_ID         0x6C1
 
 #define MPPT_A_TEMPERATURE_ID                 0x6A2
 #define MPPT_B_TEMPERATURE_ID                 0x6B2
 #define MPPT_C_TEMPERATURE_ID                 0x6C2
 
 #define MPPT_A_STATUS_ID                      0x6A5
 #define MPPT_B_STATUS_ID                      0x6B5
 #define MPPT_C_STATUS_ID                      0x6C5
 
 #define MPPT_A_POWER_CONNECTOR_ID             0x6A6
 #define MPPT_B_POWER_CONNECTOR_ID             0x6B6
 #define MPPT_C_POWER_CONNECTOR_ID             0x6C6
 
 #define MPPT_A_LIMITS_ID                      0x6A4
 #define MPPT_B_LIMITS_ID                      0x6B4
 #define MPPT_C_LIMITS_ID                      0x6C4
 
 #define MDU_FRAME_0_ID                      0x08850225
 #define MDU_FRAME_1_ID                      0x08950225
 #define MDU_FRAME_2_ID                      0x08A50225
 
 #define OBC_STATUS_ID                       0x18FF50E5
 
 // Mod values (1 means every message, other values rate-limit the Tx)
 // Our whitelist using a struct array
 
 // See https://ubcsolar26.monday.com/boards/7524367653/pulses/8642929457/posts/3979486939 for Module voltages and temps 
 // explanation for 9.
 static CanFilter_t filter_whitelist[] = {
     //         CAN ID                   MOD    COUNT
     { DRD_MOTOR_COMMAND_ID,                 4,     0               },
     { DRD_DIAGNOSTICS_ID,                   4,     0               },
     { DRD_TIME_SINCE_BOOTUP_ID,             1,     0               },
     { ECU_STATUS_ID,                        1,     0               },
     { MDI_TIME_SINCE_BOOTUP_ID,             1,     0               },
     { MDI_DIAGNOSTICS_ID,                   1,     0               },
     { STR_DIAGNOSTICS_ID,                   1,     0               },
     { STR_TIME_SINCE_BOOTUP_ID,             1,     0               },
     { BMS_VOLTAGE_SUMMARY_VOLTAGE_ID,       10,     0               },
     { BMS_MODULE_VOLTAGES_ID,               9,     0               },
     { BMS_FAULTS_ID,                        1,     0               },
     { BMS_TEMP_SUMMARY_ID,                  10,     0               },
     { BMS_PACK_HEALTH_ID,                   1,     0               },
     { BMS_MODULE_TEMPERATURES_ID,           9,     0               },
     { MPPT_A_INPUT_MEASUREMENTS_ID,         1,     0               },
     { MPPT_B_INPUT_MEASUREMENTS_ID,         1,     0               },
     { MPPT_C_INPUT_MEASUREMENTS_ID,         1,     0               },
 
     { MPPT_A_OUTPUT_MEASUREMENTS_ID,        1,     0               },
     { MPPT_B_OUTPUT_MEASUREMENTS_ID,        1,     0               },
     { MPPT_C_OUTPUT_MEASUREMENTS_ID,        1,     0               },
 
     { MPPT_A_TEMPERATURE_ID,                5,     0               },
     { MPPT_B_TEMPERATURE_ID,                5,     0               },
     { MPPT_C_TEMPERATURE_ID,                5,     0               },
     
     { MPPT_A_STATUS_ID,                     3,     0               },
     { MPPT_B_STATUS_ID,                     3,     0               },
     { MPPT_C_STATUS_ID,                     3,     0               },
 
     { MPPT_A_POWER_CONNECTOR_ID,            4,     0               },
     { MPPT_B_POWER_CONNECTOR_ID,            4,     0               },
     { MPPT_C_POWER_CONNECTOR_ID,            4,     0               },
 
     { MPPT_A_LIMITS_ID,                     10,     0               },
     { MPPT_B_LIMITS_ID,                     10,     0               },
     { MPPT_C_LIMITS_ID,                     10,     0               },
 
     { TEL_TIME_SINCE_BOOTUP_ID,             1,     0               },
     { TEL_DIAGNOSTICS_ID,                   1,     0               },
     { MDU_FRAME_0_ID,                       1,     0               },
     { MDU_FRAME_1_ID,                       5,     0               },
     { MDU_FRAME_2_ID,                       5,     0               },
     { OBC_STATUS_ID,                        1,     0               },
     { GPS_LONG_LAT_ID,                      1,     0               },
 };
 
 
 #define NUM_FILTERS (sizeof(filter_whitelist) / sizeof(filter_whitelist[0]))
 
 /**
  * @brief Determines if a received CAN message should be transmitted.
  *
  * This function iterates over our whitelist and increments the counter
  * for the matching CAN ID. A message is transmitted if its counter modulo
  * the specified mod is zero.
  *
  * @param can_id The incoming CAN ID.
  * @return true if the message should be transmitted, false otherwise.
  */
 static bool filter(uint32_t can_id)
 {
     #ifdef DEBUG
         return true;
     #else
         // TODO: Use hashmap for speed?
         for (int i = 0; i < NUM_FILTERS; i++) {
             if (filter_whitelist[i].id == can_id) {
                 if ( ( (++filter_whitelist[i].count) % filter_whitelist[i].mod ) == 0) {
                     return true;
                 }
                 break; // Found the matching ID; no need to keep looping.
             }
         }
         return false;
     #endif
 }
 
 
 /**
  * @brief Transmits a received message over UART to the XBee Radio Module
  * 
  * @param CAN_comms_Rx_msg Pointer to the CAN Rx message
  */
 RADIO_Msg_TypeDef radio_msg = {0};
 void RADIO_filter_and_queue_msg(CAN_comms_Rx_msg_t* CAN_comms_Rx_msg)
 {
     uint32_t can_id = (CAN_comms_Rx_msg->header.IDE == CAN_ID_STD)? CAN_comms_Rx_msg->header.StdId : CAN_comms_Rx_msg->header.ExtId;
 
     if (filter(can_id))
     {
         osSemaphoreAcquire(usart1_tx_semaphore, osWaitForever);   // Dont Tx until previous Tx is done
         
         /* Create radio message struct */
         set_radio_msg(&(CAN_comms_Rx_msg->header), CAN_comms_Rx_msg->data, &radio_msg);
         
         /* Transmit Radio Message */
         UART_radio_transmit(&radio_msg);
     }
 }
 
 /**
  * @brief Transmits a TEL (GPS or IMU) message over UART to the XBee Radio Module
  * 
  * @param CAN_comms_Rx_msg Pointer to the CAN Rx message
  */
 void RADIO_filter_and_queue_msg_tx(CAN_comms_Tx_msg_t* CAN_comms_Tx_msg)
 {
     uint32_t can_id = (CAN_comms_Tx_msg->header.IDE == CAN_ID_STD)? CAN_comms_Tx_msg->header.StdId : CAN_comms_Tx_msg->header.ExtId;
     
     if (filter(can_id))
     {
         osSemaphoreAcquire(usart1_tx_semaphore, osWaitForever);   // Dont Tx until previous Tx is done
         
         /* Create radio message struct */
         set_radio_msg_tx(&(CAN_comms_Tx_msg->header), CAN_comms_Tx_msg->data, &radio_msg);
 
         /* Transmit Radio Message */
         UART_radio_transmit(&radio_msg);
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
  * @brief Sets all the fields in the radio message struct
  * (int i = 0; i < NUM_FILTERS; i++) {
             if (filter_whitelist[i].id == can_id) {
                 if ( ( (++filter_whitelist[i].count) % filter_whitelist[i].mod ) == 0) {
                     return true;
                 }
                 break; // Found the matching ID; no need to keep looping.
             }
         }
         return false;
  * @param header The CAN header struct
  * @param data The CAN data
  */
 void set_radio_msg_tx(CAN_TxHeaderTypeDef* header, uint8_t* data, RADIO_Msg_TypeDef* radio_msg)
 {
     memset(radio_msg, 0, sizeof(RADIO_Msg_TypeDef));           // 0 out all 8 bytes data
     
     radio_msg->timestamp        = get_timestamp();
     uint32_t id 				= (header->IDE == CAN_ID_STD) ? header->StdId : header->ExtId; 
     radio_msg->can_id           = BITOPS_32BIT_REVERSE(id);
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