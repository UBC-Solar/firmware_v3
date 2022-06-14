#include "CAN_messages.h"

// #define MESSAGE624_SIZE 6

// #define MESSAGE624_SIZE 100

uint8_t PH_expectedMessage624[MESSAGE624_SIZE] = {0};
uint8_t PH_message624[MESSAGE624_SIZE] = {0};

short CAN_getPackCurrent()
{
    return Pack_getPackCurrent();
}

uint8_t * CAN_createMessage624()
{
    short packCurrent = CAN_getPackCurrent();

    for(int i=0; i < MESSAGE624_SIZE ; ++i)
    {
        PH_message624[i] = 0;
    }

    PH_message624[0] = (uint8_t)(packCurrent>>8);
    PH_message624[1] = (uint8_t)packCurrent;
    PH_message624[2] = 0;
    PH_message624[3] = 0;
    PH_message624[4] = 0;
    PH_message624[5] = 0;

    return PH_message624;
}



uint8_t * CAN_createExpectedMessage624(short packCurrent)
{
    for(int i=0; i<MESSAGE624_SIZE ; ++i)
    {
        PH_expectedMessage624[i] = 0;
    }

    //convert integer pack current to twos complement
    //uh, type short is already twos complement I think

    PH_expectedMessage624[0] = (uint8_t)(packCurrent>>8);
    PH_expectedMessage624[1] = (uint8_t)packCurrent;
    PH_expectedMessage624[2] = 0;
    PH_expectedMessage624[3] = 0;
    PH_expectedMessage624[4] = 0;
    PH_expectedMessage624[5] = 0;

    return PH_expectedMessage624;
}
