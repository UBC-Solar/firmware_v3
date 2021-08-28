#include "nucleo_recieve_data.h"

CAN_HandleTypeDef *hcan;
CAN_RxHeaderTypeDef *RxHeader;
CAN_FilterTypeDef *sFilterConfig;
uint8_t RxData[8];

//function from stm32f3xx_hal_can.c, called when "message pending in FIFO0" interrupt is triggered
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    static int startup = 1; //1 indicates nucleo has just been plugged in
    int incomingID;
    static int currentID;
    static int initial_message_series = 1;

    HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, RxHeader, RxData);
    //incomingID = get CAN ID from incomming message

    //startup sequence
    if (startup == 1)
    {
        if (initial_message_series == 1)
        { //first message sent over CAN
            currentID = incomingID;
            initial_message_series = 0;
        }

        if (incomingID != currentID) //new data type being sent
        {
            startup = 0; //end startup sequence
        }
    }

    if (startup == 0)
    {
        if (incomingID != currentID)
        {
            HAL_UART_Transmit(&huart2, (uint8_t*)&NEWLINE_CHAR, strlen(NEWLINE_CHAR), TIMEOUT); //signalling GUI that new data type is incoming
            //data is uint8_t*, so would this work to pass address of #defined variable?
            HAL_UART_Transmit(&huart2, (uint8_t*)hash_table_lookup(incomingID), strlen(hash_table_lookup(incomingID)), TIMEOUT);
            //implement CAN_ID_table_lookup function
            //HAL_UART_Transmit takes in pointer to data, so ensure we're passing address of data to function
            currentID = incomingID;
        }
        for (int i = 0; i < CAN_BYTES_SENT; i++)
        {
            HAL_UART_Transmit(&huart2, (uint8_t*)&(RxData[i]), sizeof(RxData[i]), TIMEOUT);
            //HAL_UART_Transmit takes in pointer to data, so ensure we're passing address of data to function
        }
    }
}
