#include "nucleo_recieve_data.h"

CAN_HandleTypeDef *hcan;
CAN_FilterTypeDef *sFilterConfig;
uint8_t RxData[8];

//function from stm32f3xx_hal_can.c, called when "message pending in FIFO0" interrupt is triggered
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    static int startup = 1; //1 indicates nucleo has just been plugged in
    int incomingID;
    static int currentID;
    static int initial_message_series = 1;

    HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &RxHeader, RxData);
    incomingID = RxHeader.StdId;

    //startup sequence to run when program first initiated
    if (startup == 1)
    {
        if (initial_message_series == 1)
        { //sets currentID = ID of first message recieved
            currentID = incomingID;
            initial_message_series = 0;
        }

        if (incomingID != currentID) //signals that a new batch of data is incoming
        {
            startup = 0; //end startup sequence
        }
    }

    if (startup == 0)
    {
        if (incomingID != currentID) //new datatype incoming
        {
            HAL_UART_Transmit(&huart2, (uint8_t *)&NEWLINE_CHAR, strlen(NEWLINE_CHAR), TIMEOUT); //newline char signals GUI that new data is incoming
            HAL_UART_Transmit(&huart2, (uint8_t *)hash_table_lookup(incomingID), strlen(hash_table_lookup(incomingID)), TIMEOUT);
            //send datatype char associated with incoming CAN ID (ex. 0x100 ID = "v")
            currentID = incomingID;
        }
        for (int i = 0; i < CAN_BYTES_SENT; i++)
        {
            HAL_UART_Transmit(&huart2, (uint8_t *)&(RxData[i]), sizeof(RxData[i]), TIMEOUT);
            //HAL_UART_Transmit takes in pointer to data, so ensure we're passing address of data to function
        }
    }
}
