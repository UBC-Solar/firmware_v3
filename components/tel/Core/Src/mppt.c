#include "mppt.h"
#include "CAN_comms.h"
#include "bitops.h"


/* CAN IDS */
#define MPPT_A_OUTPUT_VOLTAGE_MAX_ID             0x6AA
#define MPPT_B_OUTPUT_VOLTAGE_MAX_ID             0x6BA
#define MPPT_C_OUTPUT_VOLTAGE_MAX_ID             0x6CA

#define MPPT_A_INPUT_CURRENT_MAX_ID              0x6AB
#define MPPT_B_INPUT_CURRENT_MAX_ID              0x6BB
#define MPPT_C_INPUT_CURRENT_MAX_ID              0x6CB

/* CAN HEADERS */
CAN_TxHeaderTypeDef header_output_voltage_max_A = {
    .StdId = MPPT_A_OUTPUT_VOLTAGE_MAX_ID,
    .ExtId = 0x0000,
    .IDE = CAN_ID_STD,
    .RTR = CAN_RTR_DATA,
    .DLC = 4
};
CAN_TxHeaderTypeDef header_output_voltage_max_B = {
    .StdId = MPPT_B_OUTPUT_VOLTAGE_MAX_ID,
    .ExtId = 0x0000,
    .IDE = CAN_ID_STD,
    .RTR = CAN_RTR_DATA,
    .DLC = 4
};
CAN_TxHeaderTypeDef header_output_voltage_max_C = {
    .StdId = MPPT_C_OUTPUT_VOLTAGE_MAX_ID,
    .ExtId = 0x0000,
    .IDE = CAN_ID_STD,
    .RTR = CAN_RTR_DATA,
    .DLC = 4
};

CAN_TxHeaderTypeDef header_output_current_max_A = {
    .StdId = MPPT_A_INPUT_CURRENT_MAX_ID,
    .ExtId = 0x0000,
    .IDE = CAN_ID_STD,
    .RTR = CAN_RTR_DATA,
    .DLC = 4
};
CAN_TxHeaderTypeDef header_output_current_max_B = {
    .StdId = MPPT_B_INPUT_CURRENT_MAX_ID,
    .ExtId = 0x0000,
    .IDE = CAN_ID_STD,
    .RTR = CAN_RTR_DATA,
    .DLC = 4
};
CAN_TxHeaderTypeDef header_output_current_max_C = {
    .StdId = MPPT_C_INPUT_CURRENT_MAX_ID,
    .ExtId = 0x0000,
    .IDE = CAN_ID_STD,
    .RTR = CAN_RTR_DATA,
    .DLC = 4
};

/**
 * @brief Sets ALL MPPTs output voltage max. 
 * @param voltage_to_set: Value of the max output voltage to set the MPPT to via CAN
 */
void MPPT_output_voltage_max_command(float voltage_to_set)
{
    // Get the bytes for the float voltage
    FloatToBytes voltage_union;
    voltage_union.f = voltage_to_set;

    CAN_comms_Tx_msg_t voltage_max_A = {
        .data[0] = voltage_union.bytes[0], 
        .data[1] = voltage_union.bytes[1], 
        .data[2] = voltage_union.bytes[2], 
        .data[3] = voltage_union.bytes[3], 
        .header = header_output_voltage_max_A
    };
    CAN_comms_Tx_msg_t voltage_max_B = {
        .data[0] = voltage_union.bytes[0], 
        .data[1] = voltage_union.bytes[1], 
        .data[2] = voltage_union.bytes[2], 
        .data[3] = voltage_union.bytes[3], 
        .header = header_output_voltage_max_B
    };
    CAN_comms_Tx_msg_t voltage_max_C = {
        .data[0] = voltage_union.bytes[0], 
        .data[1] = voltage_union.bytes[1], 
        .data[2] = voltage_union.bytes[2], 
        .data[3] = voltage_union.bytes[3], 
        .header = header_output_voltage_max_C
    };

    CAN_comms_Add_Tx_message(&voltage_max_A);
    osDelay(3);
    CAN_comms_Add_Tx_message(&voltage_max_B);
    osDelay(3);
    CAN_comms_Add_Tx_message(&voltage_max_C);
    osDelay(3);
}


/**
 * @brief Sets ALL MPPTs input current max. 
 * @param voltage_to_set: Value of the max input current to set the MPPT to via CAN
 */
void MPPT_input_current_max_command(float current_to_set)
{
    // Get the bytes for the float voltage
    FloatToBytes current_union;
    current_union.f = current_to_set;
    
    CAN_comms_Tx_msg_t current_max_A = {
        .data[0] = current_union.bytes[0], 
        .data[1] = current_union.bytes[1], 
        .data[2] = current_union.bytes[2], 
        .data[3] = current_union.bytes[3], 
        .header = header_output_current_max_A
    };
    CAN_comms_Tx_msg_t current_max_B = {
        .data[0] = current_union.bytes[0], 
        .data[1] = current_union.bytes[1], 
        .data[2] = current_union.bytes[2], 
        .data[3] = current_union.bytes[3], 
        .header = header_output_current_max_B
    };
    CAN_comms_Tx_msg_t current_max_C = {
        .data[0] = current_union.bytes[0], 
        .data[1] = current_union.bytes[1], 
        .data[2] = current_union.bytes[2], 
        .data[3] = current_union.bytes[3], 
        .header = header_output_current_max_C
    };
    
    CAN_comms_Add_Tx_message(&current_max_A);
    osDelay(3);
    CAN_comms_Add_Tx_message(&current_max_B);
    osDelay(3);
    CAN_comms_Add_Tx_message(&current_max_C);
    osDelay(3);
}
