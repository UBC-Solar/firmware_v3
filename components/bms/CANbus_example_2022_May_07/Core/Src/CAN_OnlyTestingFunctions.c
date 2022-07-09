#include "CAN_OnlyTestingFunctions.h"

// #define MESSAGE624_SIZE 6

// #define MESSAGE624_SIZE 100

uint8_t expectedMessage624[MESSAGE624_SIZE] = {0};
uint8_t message624[MESSAGE624_SIZE] = {0};

uint8_t expectedMessage622[MESSAGE622_SIZE] = {0};
uint8_t expectedMessage623[MESSAGE623_SIZE] = {0};
uint8_t expectedMessage626[MESSAGE626_SIZE] = {0};
uint8_t expectedMessage627[MESSAGE627_SIZE] = {0};

short CAN_getPackCurrent()
{
    return Pack_getPackCurrent();
}

uint8_t * CAN_createMessage624()
{
    short packCurrent = CAN_getPackCurrent();

    for(int i=0; i < MESSAGE624_SIZE ; ++i)
    {
        message624[i] = 0;
    }

    message624[0] = (uint8_t)(packCurrent>>8);
    message624[1] = (uint8_t)packCurrent;
    message624[2] = 0;
    message624[3] = 0;
    message624[4] = 0;
    message624[5] = 0;

    return message624;
}






uint8_t * CAN_createExpectedMessage622withFaultFlag(uint8_t faultFlag)
{
    for(int i=0; i<MESSAGE622_SIZE ; ++i)
    {
        expectedMessage622[i] = 0;
    }

    // expectedMessage622[0] = 0; // bits 0 to 7
    // expectedMessage622[1] = 0; // bits 8 to 15
    // expectedMessage622[2] = 0; // bits 16 to 23
    // expectedMessage622[3] = 0; // bits 24 to 31
    expectedMessage622[4] = faultFlag; // bits 32 to 39
    // expectedMessage622[5] = 0; // bits 40 to 47
    // expectedMessage622[6] = 0; // bits 48 to 55

    return expectedMessage622;
}

uint8_t * CAN_createExpectedMessage622withBitFlag(uint8_t bitNumber)
{
    uint64_t longBinary = 0ULL;

    for(int i=0; i<MESSAGE622_SIZE ; ++i)
    {
        expectedMessage622[i] = 0;
    }

    if(
            (bitNumber == 0)
        ||  (bitNumber == 29)
        ||  (bitNumber == 30)
        ||  (bitNumber >= 41 && bitNumber <= 55)
    )
    {
        longBinary = longBinary & (1ULL << (63-bitNumber));
    }

    expectedMessage622[0] = (uint8_t)(longBinary >> 48); // bits 0 to 7
    expectedMessage622[1] = (uint8_t)(longBinary >> 40); // bits 8 to 15
    expectedMessage622[2] = (uint8_t)(longBinary >> 32); // bits 16 to 23
    expectedMessage622[3] = (uint8_t)(longBinary >> 24); // bits 24 to 31
    expectedMessage622[4] = (uint8_t)(longBinary >> 16); // bits 32 to 39
    expectedMessage622[5] = (uint8_t)(longBinary >> 8); // bits 40 to 47
    expectedMessage622[6] = (uint8_t)longBinary; // bits 48 to 55

    return expectedMessage622;
}

uint8_t * CAN_createExpectedMessage622withTime(uint16_t time)
{
    for(int i=0; i<MESSAGE622_SIZE ; ++i)
    {
        expectedMessage622[i] = 0;
    }

    // expectedMessage622[0] = 0; // bits 0 to 7
    expectedMessage622[1] = (uint8_t)(time >> 8); // bits 8 to 15
    expectedMessage622[2] = (uint8_t)time; // bits 16 to 23
    // expectedMessage622[3] = 0; // bits 24 to 31
    // expectedMessage622[4] = 0; // bits 32 to 39
    // expectedMessage622[5] = 0; // bits 40 to 47
    // expectedMessage622[6] = 0; // bits 48 to 55

    return expectedMessage622;
}




uint8_t * CAN_createExpectedMessage624(short packCurrent)
{
    for(int i=0; i<MESSAGE624_SIZE ; ++i)
    {
        expectedMessage624[i] = 0;
    }

    //convert integer pack current to twos complement
    //uh, type short is already twos complement I think

    expectedMessage624[0] = (uint8_t)(packCurrent>>8);
    expectedMessage624[1] = (uint8_t)packCurrent;
    expectedMessage624[2] = 0;
    expectedMessage624[3] = 0;
    expectedMessage624[4] = 0;
    expectedMessage624[5] = 0;

    return expectedMessage624;
}

uint8_t * CAN_createIdealMessage622withFaultFlag(uint8_t faultFlag)
{
    return CAN_createExpectedMessage622withFaultFlag(faultFlag);
}

uint8_t * CAN_createIdealMessage622withBitFlag(uint8_t bitNumber)
{
    return CAN_createExpectedMessage622withBitFlag(bitNumber);
}

uint8_t * CAN_createIdealMessage622withTime(uint16_t time)
{
    return CAN_createExpectedMessage622withTime(time);
}



uint8_t * CAN_createExpectedMessage623
(
    uint16_t totalPackVoltage,
    uint8_t lowestChargedVoltage,
    uint8_t IDofLowestVoltageCell,
    uint8_t highestChargedVoltage,
    uint8_t IDofHighestVoltageCell
)
{
    for(int i=0; i<MESSAGE623_SIZE ; ++i)
    {
        expectedMessage623[i] = 0;
    }


    expectedMessage623[0] = (uint8_t)(totalPackVoltage>>8); //bits 0-7
    expectedMessage623[1] = (uint8_t)totalPackVoltage;      //bits 8-15
    expectedMessage623[2] = lowestChargedVoltage;           //bits 16-23
    expectedMessage623[3] = IDofLowestVoltageCell;          //bits 24-31
    expectedMessage623[4] = highestChargedVoltage;          //bits 32-39
    expectedMessage623[5] = IDofHighestVoltageCell;         //bits 40-47

    return expectedMessage623;
}


uint8_t * CAN_createExpectedMessage626
(
    uint8_t StateOfCharge,
    uint16_t DepthOfDischarge,
    uint16_t Capacity
)
{
    for(int i=0; i<MESSAGE626_SIZE ; ++i)
    {
        expectedMessage626[i] = 0;
    }


    expectedMessage626[0] = StateOfCharge;                  //bits 0-7
    expectedMessage626[1] = (uint8_t)(DepthOfDischarge>>8); //bits 8-15
    expectedMessage626[2] = (uint8_t)DepthOfDischarge;      //bits 16-23
    expectedMessage626[3] = (uint8_t)(Capacity>>8);         //bits 24-31
    expectedMessage626[4] = (uint8_t)Capacity;              //bits 32-39
    // expectedMessage625[5] = 0;                           //bits 40-47
    // expectedMessage625[6] = 0;                           //bits 48-55

    return expectedMessage626;
}

uint8_t * CAN_createExpectedMessage627
(
    uint8_t temperature,
    uint8_t minTemp,
    uint8_t IDofMinTempCell,
    uint8_t maxTemp,
    uint8_t IDofMaxTempCell
)
{
    for(int i=0; i<MESSAGE627_SIZE ; ++i)
    {
        expectedMessage627[i] = 0;
    }


    expectedMessage627[0] = temperature;     //bits 0-7
    expectedMessage627[1] = 0;               //bits 8-15
    expectedMessage627[2] = minTemp;         //bits 16-23
    expectedMessage627[3] = IDofMinTempCell; //bits 24-31
    expectedMessage627[4] = maxTemp;         //bits 32-39
    expectedMessage627[5] = IDofMaxTempCell; //bits 40-47

    return expectedMessage626;
}
