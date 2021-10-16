#include "nucleo_recieve_data.h"

//function from stm32f3xx_hal_can.c, called when "message pending in FIFO0" interrupt is triggered
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    CAN_RxHeaderTypeDef RxHeader;
    uint8_t RxData[8];

    //characters GUI uses to classify type of incomming data
    //PLACEHOLDER VALUES - pending Matthew's CAN IDs
    ID_table_t volt_id = {.can_id = 0x100, .datatype = "v"};
    ID_table_t temp_id = {.can_id = 0x200, .datatype = "t"};
    ID_table_t soc_id = {.can_id = 0x300, .datatype = "c"};
    ID_table_t mod_status_id = {.can_id = 0x400, .datatype = "s"};
    ID_table_t pack_status_id = {.can_id = 0x500, .datatype = "y"};

    static int startup = 1; //1 indicates nucleo has just been plugged in
    int incomingID;
    static int currentID;
    static int initial_message_series = 1;
    char incomingID_char;

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
            if (incomingID == volt_id.can_id)
            {
                incomingID_char = volt_id.datatype;
            }
            else if (incomingID == temp_id.can_id)
            {
                incomingID_char = temp_id.datatype;
            }
            else if (incomingID == soc_id.can_id)
            {
                incomingID_char = soc_id.datatype;
            }
            else if (incomingID == mod_status_id.can_id)
            {
                incomingID_char = mod_status_id.datatype;
            }
            else if (incomingID == pack_status_id.can_id)
            {
                incomingID_char = pack_status_id.datatype;
            }

            HAL_UART_Transmit(&huart2, (uint8_t *)&NEWLINE_CHAR, strlen(NEWLINE_CHAR), TIMEOUT);        //newline char signals GUI that new data is incoming
            HAL_UART_Transmit(&huart2, (uint8_t *)(incomingID_char), strlen(incomingID_char), TIMEOUT); //send datatype char associated with incoming CAN ID (ex. 0x100 ID = "v")
            currentID = incomingID;
        }
        for (int i = 0; i < CAN_BYTES_SENT; i++)
        {
            HAL_UART_Transmit(&huart2, (uint8_t *)&(RxData[i]), sizeof(RxData[i]), TIMEOUT);
            //HAL_UART_Transmit takes in pointer to data, so ensure we're passing address of data to function
        }
    }
}
