#include "diagnostic.h"
#include "stdint.h"
#include "CAN_comms.h"
#include "radio.h"

TEL_Diagnostic_Flags_t g_tel_diagnostic_flags = {0};
volatile uint32_t g_time_since_bootup;

#define TIME_SINCE_BOOTUP_CAN_DATA_LENGTH          4       
#define TIME_SINCE_BOOTUP_CAN_ID                   0x750

#define TEL_FLAGS_CAN_DATA_LENGTH                  1       
#define TEL_FLAGS_BOOTUP_CAN_ID                    0x751

static CAN_TxHeaderTypeDef time_since_bootup_can_header = {
    .StdId = TIME_SINCE_BOOTUP_CAN_ID,
    .ExtId = 0x0000,
    .IDE = CAN_ID_STD,
    .RTR = CAN_RTR_DATA,
    .DLC = TIME_SINCE_BOOTUP_CAN_DATA_LENGTH};

static CAN_TxHeaderTypeDef tel_flags_can_header = {
    .StdId = TEL_FLAGS_BOOTUP_CAN_ID,
    .ExtId = 0x0000,
    .IDE = CAN_ID_STD,
    .RTR = CAN_RTR_DATA,
    .DLC = TEL_FLAGS_CAN_DATA_LENGTH};


/**
  * @brief  Sends the time since bootup via CAN and radio tx.
  * @retval None
*/
void send_time_since_bootup_can_radio()
{
    CAN_comms_Tx_msg_t time_since_bootup_can_tx = {
        .data[0] = g_time_since_bootup & 0x000000FFU,
        .data[1] = g_time_since_bootup & 0x0000FF00U,
        .data[2] = g_time_since_bootup & 0x00FF0000U,
        .data[3] = g_time_since_bootup & 0xFF000000U,
        .header = time_since_bootup_can_header,
    };

    CAN_comms_Add_Tx_message(&time_since_bootup_can_tx);
    osDelay(3);
    RADIO_filter_and_queue_msg_tx(&time_since_bootup_can_tx);
}

/**
  * @brief  Sends tel flags via CAN
  * @retval None
  */
void send_tel_flags_can()
{
    CAN_comms_Tx_msg_t tel_flags_can_tx = {
        .data[0] = g_tel_diagnostic_flags.raw,
        .header = tel_flags_can_header,
    };

    CAN_comms_Add_Tx_message(&tel_flags_can_tx);
    osDelay(3);
}

/**
  * @brief  Sends tel diagnostic flags (can) and the time since boot up (can, radio)
  * @retval None
  */
void DIAGNOSTIC_send_can()
{
    send_time_since_bootup_can_radio();
    send_tel_flags_can();
}

/**
  * @brief  Reset all flags and values
  * @retval None
  */
void DIAGNOSTIC_init()
{
    // Initilaize diagnostic flags as cleared at first
    g_tel_diagnostic_flags.raw = false;
    g_time_since_bootup = 0;
}
