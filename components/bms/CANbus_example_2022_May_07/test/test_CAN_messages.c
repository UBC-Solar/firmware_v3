#ifdef TEST

// #define ENABLE_PRINTF_MESSAGES

#include "unity.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "mock_Pack_packdata.h"
// #include "CAN_OnlyTestingFunctions.h"
#include "CAN_OnlyTestingFunctions.h"

// #include "mock_CAN_messages.h"
//#include "mock_stm32f103xb.h"
//#include "mock_stm32f1xx.h"
//#include "stm32f1xx_hal.h"
//#include "stm32f1xx_hal_conf.h"
//#include "mock_core_cm3.h"
#include "mock_stm32f1xx_hal.h"
#include "mock_ltc6813_btm.h"
#include "mock_analysis.h"
#include "mock_stm32f1xx_hal_can.h"

#include "CANbus_functions.h"

uint16_t TEST_PACK_CURRENT = 7;

void hello_world(int i);

void FOR_message623_assert_equal
(
    voltageInfoStruct * voltageInfoStructPtr,

    uint16_t ActualTotalVoltage,

    uint16_t ActualMinVoltage,
    uint8_t ActualMinStackIndex,
    uint8_t ActualMinModuleIndex,
    uint16_t ActualMaxVoltage,
    uint8_t ActualMaxStackIndex,
    uint8_t ActualMaxModuleIndex,

    uint16_t ExpectedTotalVoltage,

    uint8_t ExpectedMinVoltage,
    uint8_t ExpectedMinVoltageSticker,
    uint8_t ExpectedMaxVoltage,
    uint8_t ExpectedMaxVoltageSticker
);

void compareArraysOfSize6(uint8_t expectedArray[6], uint8_t actualArray[6]);
void compareArraysOfSize7(uint8_t* expectedArray, uint8_t* actualArray);
void compareArraysOfSize8(uint8_t* expectedArray, uint8_t* actualArray);


// typedef struct voltageInfoStruct{
//     uint16_t MinVoltage;
//     uint16_t MaxVoltage;
//     uint8_t MinStackIndex;
//     uint8_t MinModuleIndex;
//     uint8_t MaxStackIndex;
//     uint8_t MaxModuleIndex;
// }voltageInfoStruct;

voltageInfoStruct Placeholder_voltageInfo;



void setUp(void)
{
    Placeholder_voltageInfo.MinVoltage = 0;
    Placeholder_voltageInfo.MaxVoltage = 0;
    Placeholder_voltageInfo.MinStackIndex = 0;
    Placeholder_voltageInfo.MinModuleIndex = 0;
    Placeholder_voltageInfo.MaxStackIndex = 0;
    Placeholder_voltageInfo.MaxModuleIndex = 0;
}

void tearDown(void)
{
}


/**
Purpose:
        test message with ID 0x624 that it reports a matching current in the
        expected format
*/
void test_message624_currentStatus()
{
    short electricCurrent;

    uint8_t * expectedDataFramePtr;
    uint8_t * actualDataFramePtr;


    //Test 1
    electricCurrent = 123;

    // CAN_getPackCurrent_ExpectAndReturn(electricCurrent);
    Pack_getPackCurrent_ExpectAndReturn(electricCurrent);
    expectedDataFramePtr = CAN_createExpectedMessage624(electricCurrent);
    actualDataFramePtr = CAN_createMessage624();

//     for(int i = 0; i < MESSAGE624_SIZE; ++i)
//     {
//         TEST_ASSERT_EQUAL(expectedDataFramePtr[i], actualDataFramePtr[i]);
//     }

#ifdef ENABLE_PRINTF_MESSAGES
    for(int i = 0; i < MESSAGE624_SIZE; ++i)
    {
            printf("%i\r\n",expectedDataFramePtr[i]);
    }

    for(int i = 0; i < MESSAGE624_SIZE; ++i)
    {
            printf("%i\r\n",actualDataFramePtr[i]);
    }
#endif

    TEST_ASSERT_EQUAL(expectedDataFramePtr[0], actualDataFramePtr[0]);
    TEST_ASSERT_EQUAL(expectedDataFramePtr[1], actualDataFramePtr[1]);
    TEST_ASSERT_EQUAL(expectedDataFramePtr[2], actualDataFramePtr[2]);
    TEST_ASSERT_EQUAL(expectedDataFramePtr[3], actualDataFramePtr[3]);
    TEST_ASSERT_EQUAL(expectedDataFramePtr[4], actualDataFramePtr[4]);
    TEST_ASSERT_EQUAL(expectedDataFramePtr[5], actualDataFramePtr[5]);


    //Test 2
    electricCurrent = 111;
    Pack_getPackCurrent_IgnoreAndReturn(electricCurrent);
    expectedDataFramePtr = CAN_createExpectedMessage624(electricCurrent);
    actualDataFramePtr = CAN_createMessage624(electricCurrent);

    for(int i = 0; i < MESSAGE624_SIZE; ++i)
    {
            printf("%i\r\n",expectedDataFramePtr[i]);
    }

    for(int i = 0; i < MESSAGE624_SIZE; ++i)
    {
            printf("%i\r\n",actualDataFramePtr[i]);
    }

    // for(int i = 0; i < MESSAGE624_SIZE; ++i)
    // {
    //     TEST_ASSERT_EQUAL(expectedDataFramePtr[i], actualDataFramePtr[i]);
    // }
    TEST_ASSERT_EQUAL(expectedDataFramePtr[0], actualDataFramePtr[0]);
    TEST_ASSERT_EQUAL(expectedDataFramePtr[1], actualDataFramePtr[1]);
    TEST_ASSERT_EQUAL(expectedDataFramePtr[2], actualDataFramePtr[2]);
    TEST_ASSERT_EQUAL(expectedDataFramePtr[3], actualDataFramePtr[3]);
    TEST_ASSERT_EQUAL(expectedDataFramePtr[4], actualDataFramePtr[4]);
    TEST_ASSERT_EQUAL(expectedDataFramePtr[5], actualDataFramePtr[5]);

}

/**
Purpose:
    Test message with ID 0x622 that sets all flags in the right format.
    DOES NOT test for proper flag raising during specific errors. Not yet
    at least.

Algorithm:
    A) Test fault codes at bits 32-39.
        1) set message to all zero
        2) in a for loop, test that all fault codes are sent properly
            2.1) use createMessage622WithFaultCode(), with printf
                 to check which fault code fails
    B) Test every single fault and warning bit flag, one at a time.
        1) set message to all zero
        2) in a for loop, test that all fault codes are sent properly
            2.1) use createMessage622WithBitFlag(), with printf
                 to check which fault code fails
    C) Test the time-after-power-on at bits 8-23
*/
void test_message622_faultsMessage_formatVerification()
{
    uint8_t* expectedMessage = NULL;
    uint8_t* actualMessage = NULL;
    int expectedSize = 0;
    int actualSize = 0;

    const char * debugStrings_faults[64];
    debugStrings_faults[0]  = "Bit  0: BMS fault bit";
    debugStrings_faults[29] = "Bit 29: HLIM is set: cannot charge";
    debugStrings_faults[30] = "Bit 30: LLIM is set: cannot discharge";
    debugStrings_faults[41] = "Bit 41: Interlock is tripped";
    debugStrings_faults[42] = "Bit 42: Communication fault with cell";
    debugStrings_faults[43] = "Bit 43: Charge overcurrent";
    debugStrings_faults[44] = "Bit 44: Discharge overcurrent";
    debugStrings_faults[45] = "Bit 45: Over temperature";
    debugStrings_faults[46] = "Bit 46: Under voltage";
    debugStrings_faults[47] = "Bit 47: Over voltage";
    debugStrings_faults[48] = "Bit 48: Low voltage warning";
    debugStrings_faults[49] = "Bit 49: High voltage warning";
    debugStrings_faults[50] = "Bit 50: Charge overcurrent warning";
    debugStrings_faults[51] = "Bit 51: Discharge overcurrent warning";
    debugStrings_faults[52] = "Bit 52: Cold temperature warning";
    debugStrings_faults[53] = "Bit 53: Hot temperature warning";
    debugStrings_faults[54] = "Bit 54: Low SOH warning";
    debugStrings_faults[55] = "Bit 55: Isolation fault warning";



    //testing all fault flags
    for(int i; i<= 19; ++i)
    {
        expectedMessage = CAN_createExpectedMessage622withFaultFlag(i);
        //INSERT MOCK CALLS HERE
        actualMessage   = CAN_createIdealMessage622withFaultFlag(i);

        //check size of messages
        expectedSize = sizeof(expectedMessage)/sizeof(expectedMessage[0]);
        actualSize   = sizeof(actualMessage)  /sizeof(actualMessage[0]);

        //compare message contents
        TEST_ASSERT_EQUAL(expectedMessage[0], actualMessage[0]);
        TEST_ASSERT_EQUAL(expectedMessage[1], actualMessage[1]);
        TEST_ASSERT_EQUAL(expectedMessage[2], actualMessage[2]);
        TEST_ASSERT_EQUAL(expectedMessage[3], actualMessage[3]);
        TEST_ASSERT_EQUAL(expectedMessage[4], actualMessage[4]);
        TEST_ASSERT_EQUAL(expectedMessage[5], actualMessage[5]);
        TEST_ASSERT_EQUAL(expectedMessage[6], actualMessage[6]);
    }



    ///testing all bit flags
    for(int i; i < 64; ++i)
    {
        int bitNumber = i;
        if(
                (bitNumber == 0)
            ||  (bitNumber == 29)
            ||  (bitNumber == 30)
            ||  (bitNumber >= 41 && bitNumber <= 55)
        )
        {
            printf("iter %2.0i,  %s \r\n", i, debugStrings_faults[i]);
            expectedMessage = CAN_createExpectedMessage622withBitFlag(i);
            //INSERT MOCK CALLS HERE
            actualMessage   = CAN_createIdealMessage622withBitFlag(i);

            //check size of messages
            expectedSize = sizeof(expectedMessage)/sizeof(expectedMessage[0]);
            actualSize   = sizeof(actualMessage)  /sizeof(actualMessage[0]);

            TEST_ASSERT_EQUAL(expectedSize, actualSize);

            {//compare message contents
                TEST_ASSERT_EQUAL(expectedMessage[0], actualMessage[0]);
                TEST_ASSERT_EQUAL(expectedMessage[1], actualMessage[1]);
                TEST_ASSERT_EQUAL(expectedMessage[2], actualMessage[2]);
                TEST_ASSERT_EQUAL(expectedMessage[3], actualMessage[3]);
                TEST_ASSERT_EQUAL(expectedMessage[4], actualMessage[4]);
                TEST_ASSERT_EQUAL(expectedMessage[5], actualMessage[5]);
                TEST_ASSERT_EQUAL(expectedMessage[6], actualMessage[6]);
            }
        }
    }

    //testing time field with just zero
    expectedMessage = CAN_createExpectedMessage622withTime(0);
    //INSERT MOCK CALLS HERE
    actualMessage   = CAN_createIdealMessage622withTime(0);

    //check size of messages
    expectedSize = sizeof(expectedMessage)/sizeof(expectedMessage[0]);
    actualSize   = sizeof(actualMessage)  /sizeof(actualMessage[0]);
    TEST_ASSERT_EQUAL(expectedSize, actualSize);

    {//compare message contents
        TEST_ASSERT_EQUAL(expectedMessage[0], actualMessage[0]);
        TEST_ASSERT_EQUAL(expectedMessage[1], actualMessage[1]);
        TEST_ASSERT_EQUAL(expectedMessage[2], actualMessage[2]);
        TEST_ASSERT_EQUAL(expectedMessage[3], actualMessage[3]);
        TEST_ASSERT_EQUAL(expectedMessage[4], actualMessage[4]);
        TEST_ASSERT_EQUAL(expectedMessage[5], actualMessage[5]);
        TEST_ASSERT_EQUAL(expectedMessage[6], actualMessage[6]);
    }

    //testing time field with 100 seconds
    expectedMessage = CAN_createExpectedMessage622withTime(100);
    //INSERT MOCK CALLS HERE
    actualMessage   = CAN_createIdealMessage622withTime(100);

    //check size of messages
    expectedSize = sizeof(expectedMessage)/sizeof(expectedMessage[0]);
    actualSize   = sizeof(actualMessage)  /sizeof(actualMessage[0]);
    TEST_ASSERT_EQUAL(expectedSize, actualSize);

    {//compare message contents
        TEST_ASSERT_EQUAL(expectedMessage[0], actualMessage[0]);
        TEST_ASSERT_EQUAL(expectedMessage[1], actualMessage[1]);
        TEST_ASSERT_EQUAL(expectedMessage[2], actualMessage[2]);
        TEST_ASSERT_EQUAL(expectedMessage[3], actualMessage[3]);
        TEST_ASSERT_EQUAL(expectedMessage[4], actualMessage[4]);
        TEST_ASSERT_EQUAL(expectedMessage[5], actualMessage[5]);
        TEST_ASSERT_EQUAL(expectedMessage[6], actualMessage[6]);
    }

    //testing time field with max size
    expectedMessage = CAN_createExpectedMessage622withTime(65535);
    //INSERT MOCK CALLS HERE
    actualMessage   = CAN_createIdealMessage622withTime(65535);

    //check size of messages
    expectedSize = sizeof(expectedMessage)/sizeof(expectedMessage[0]);
    actualSize   = sizeof(actualMessage)  /sizeof(actualMessage[0]);
    TEST_ASSERT_EQUAL(expectedSize, actualSize);

    {//compare message contents
        TEST_ASSERT_EQUAL(expectedMessage[0], actualMessage[0]);
        TEST_ASSERT_EQUAL(expectedMessage[1], actualMessage[1]);
        TEST_ASSERT_EQUAL(expectedMessage[2], actualMessage[2]);
        TEST_ASSERT_EQUAL(expectedMessage[3], actualMessage[3]);
        TEST_ASSERT_EQUAL(expectedMessage[4], actualMessage[4]);
        TEST_ASSERT_EQUAL(expectedMessage[5], actualMessage[5]);
        TEST_ASSERT_EQUAL(expectedMessage[6], actualMessage[6]);
    }

    //test min time, min + 100 time, and max time
}


/**

\brief  Function Name: FOR_message623_assert_equal
        Purpose: Test the compilation function for CAN message623, by creating
                 both an actual and an expected message with given parameters.

\par An ideallized formatting of this function, for ease of readability and
    compactness of code:

FOR_message623_assert_equal
(
    voltageInfoStructPtr,
//  total, min V,  labels ,  max V,  labels ,
    00000, 00000,  000,000,  00000,  000,000, //actual input data
    00000, 000,    000,      000,    000      //expected values
);
*/


void FOR_message623_assert_equal
(
    voltageInfoStruct * voltageInfoStructPtr,

    uint16_t ActualTotalVoltage,

    uint16_t ActualMinVoltage,
    uint8_t ActualMinStackIndex,
    uint8_t ActualMinModuleIndex,
    uint16_t ActualMaxVoltage,
    uint8_t ActualMaxStackIndex,
    uint8_t ActualMaxModuleIndex,

    uint16_t ExpectedTotalVoltage,

    uint8_t ExpectedMinVoltage,
    uint8_t ExpectedMinVoltageSticker,
    uint8_t ExpectedMaxVoltage,
    uint8_t ExpectedMaxVoltageSticker
)
{
    uint8_t* expectedMessage = NULL;
    uint8_t* actualMessage = NULL;

    //test all minimal values (0 or 1)
    //expected outcome: messages are successfully compiled

    //function under test: CAN_CompileMessage623()
    Pack_getVoltageInfo_IgnoreAndReturn(voltageInfoStructPtr);
    NONMOCKVERSION_setVoltageInfo
    (
        voltageInfoStructPtr,
        ActualMinVoltage,
        ActualMaxVoltage,
        ActualMinStackIndex,
        ActualMinModuleIndex,
        ActualMaxStackIndex,
        ActualMaxModuleIndex
    );
    Pack_getPackVoltage_IgnoreAndReturn(ActualTotalVoltage);

    CAN_CompileMessage623(PH_message623);

    //input 3 and 5 only go down to value 1, because battery stickers start at 1
    expectedMessage = CAN_createExpectedMessage623
    (
        ExpectedTotalVoltage,
        ExpectedMinVoltage,
        ExpectedMinVoltageSticker,
        ExpectedMaxVoltage,
        ExpectedMaxVoltageSticker
    );
    actualMessage = PH_message623;

    printf("Start of new comparison.\r\n");
    for(int i = 0; i < MESSAGE623_SIZE; ++i)
    {
        printf("expected: %5.2i, actual %5.2i\r\n", expectedMessage[i], actualMessage[i]);
    }

    // compareArraysOfSize8(expectedMessage, actualMessage);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedMessage, actualMessage, MESSAGE623_SIZE);

    return;
}

void test_message623_voltageStatus_all_mins()
{
    voltageInfoStruct* voltageInfoStructPtr = &Placeholder_voltageInfo;

    //test all min values
    FOR_message623_assert_equal
    (
        voltageInfoStructPtr,
    //  total, min V,  labels ,  max V,  labels ,
        00000, 00000,  000,000,  00000,  000,000, //actual input data
        00000, 000,    1,        000,    1        //expected values
    );
}

void test_message623_voltageStatus_all_maxs()
{
    voltageInfoStruct* voltageInfoStructPtr = &Placeholder_voltageInfo;

    //test all max values
    FOR_message623_assert_equal
    (
        voltageInfoStructPtr,
    //  total, min V,  labels ,  max V,  labels ,
        65535, 65535,  1,  17,   65535,  1  , 17, //actual input data
        65535, 65,     94,       65,     94       //expected values
    );
}

void FOR_message626_assert_equal
(
    uint8_t StateOfCharge,
    uint16_t DepthOfDischarge,
    uint16_t Capacity
)
{
    uint8_t* expectedMessage = NULL;
    uint8_t* actualMessage = NULL;

    expectedMessage = CAN_createExpectedMessage626(StateOfCharge,DepthOfDischarge,Capacity);

    Pack_getSOC_IgnoreAndReturn(StateOfCharge);
    Pack_getDOD_IgnoreAndReturn(DepthOfDischarge);
    Pack_getCapacity_IgnoreAndReturn(Capacity);
    
    CAN_CompileMessage626(PH_message626);
    actualMessage = PH_message626;

    printf("message626.\r\n");
    for(int i = 0; i < MESSAGE626_SIZE; ++i)
    {
        printf("expected: %5.2i, actual %5.2i\r\n", expectedMessage[i], actualMessage[i]);
    }

    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedMessage, actualMessage, MESSAGE626_SIZE);
}

void test_message626_packHealth_AllZeros()
{
    FOR_message626_assert_equal(0,0,0);
}

void test_message626_packHealth_AllMax()
{
    FOR_message626_assert_equal(255,65535,65535);
}


void test_message627_Temperature()
{
    uint8_t* expectedMessage = NULL;
    uint8_t* actualMessage = NULL;

    expectedMessage = CAN_createExpectedMessage627(255,255,255,255,255);
}

void compareArraysOfSize6(uint8_t expectedArray[6], uint8_t actualArray[6])
{
    uint32_t expectedSize = 0;
    uint32_t actualSize = 0;

    //check size of arrays
    // expectedSize = sizeof(*expectedArray)/sizeof(expectedArray[0]);
    // actualSize   = sizeof(*actualArray)  /sizeof(actualArray[0]);
    // TEST_ASSERT_EQUAL(expectedSize, actualSize);
    // printf("actualSize %i\r\n",actualSize);
    // printf("actualArray %i\r\n",sizeof(*actualArray));
    // printf("actualArray[0] %i\r\n",sizeof(actualArray[0]));
    // TEST_ASSERT_EQUAL(6, actualSize);
    // TEST_ASSERT_EQUAL(6, expectedSize);

    {//compare array contents
        TEST_ASSERT_EQUAL(expectedArray[0], actualArray[0]);
        TEST_ASSERT_EQUAL(expectedArray[1], actualArray[1]);
        TEST_ASSERT_EQUAL(expectedArray[2], actualArray[2]);
        TEST_ASSERT_EQUAL(expectedArray[3], actualArray[3]);
        TEST_ASSERT_EQUAL(expectedArray[4], actualArray[4]);
        TEST_ASSERT_EQUAL(expectedArray[5], actualArray[5]);
    }
}

void compareArraysOfSize7(uint8_t* expectedArray, uint8_t* actualArray)
{
    int expectedSize = 0;
    int actualSize = 0;

    //check size of arrays
    // expectedSize = sizeof(expectedArray)/sizeof(expectedArray[0]);
    // actualSize   = sizeof(actualArray)  /sizeof(actualArray[0]);
    // TEST_ASSERT_EQUAL(expectedSize, actualSize);
    // TEST_ASSERT_EQUAL(7, actualSize);
    // TEST_ASSERT_EQUAL(7, expectedSize);

    {//compare array contents
        TEST_ASSERT_EQUAL(expectedArray[0], actualArray[0]);
        TEST_ASSERT_EQUAL(expectedArray[1], actualArray[1]);
        TEST_ASSERT_EQUAL(expectedArray[2], actualArray[2]);
        TEST_ASSERT_EQUAL(expectedArray[3], actualArray[3]);
        TEST_ASSERT_EQUAL(expectedArray[4], actualArray[4]);
        TEST_ASSERT_EQUAL(expectedArray[5], actualArray[5]);
        TEST_ASSERT_EQUAL(expectedArray[6], actualArray[6]);
    }
}

void compareArraysOfSize8(uint8_t* expectedArray, uint8_t* actualArray)
{
    int expectedSize = 0;
    int actualSize = 0;

    //check size of arrays
    // expectedSize = sizeof(expectedArray)/sizeof(expectedArray[0]);
    // actualSize   = sizeof(actualArray)  /sizeof(actualArray[0]);
    // TEST_ASSERT_EQUAL(expectedSize, actualSize);
    // TEST_ASSERT_EQUAL(7, actualSize);
    // TEST_ASSERT_EQUAL(7, expectedSize);

    {//compare array contents
        TEST_ASSERT_EQUAL(expectedArray[0], actualArray[0]);
        TEST_ASSERT_EQUAL(expectedArray[1], actualArray[1]);
        TEST_ASSERT_EQUAL(expectedArray[2], actualArray[2]);
        TEST_ASSERT_EQUAL(expectedArray[3], actualArray[3]);
        TEST_ASSERT_EQUAL(expectedArray[4], actualArray[4]);
        TEST_ASSERT_EQUAL(expectedArray[5], actualArray[5]);
        TEST_ASSERT_EQUAL(expectedArray[6], actualArray[6]);
        TEST_ASSERT_EQUAL(expectedArray[7], actualArray[7]);
    }
}

void hello_world(int i)
{
    if(i == 1)
        printf("hello world, my name is Edward\r\n");
}



/**
Purpose:

Other notes: This test was only made to ensure that Ceedling mocking actually works.
*/
void test_CAN_staleCheck()
{
        uint8_t expectValue = CAN_STALE;
        HAL_CAN_GetTxMailboxesFreeLevel_IgnoreAndReturn(expectValue);
        uint8_t actualValue = CAN_staleCheck();

        TEST_ASSERT_EQUAL_UINT8(expectValue,actualValue);
}

// void test_CAN_messages_NeedToImplement(void)
// {
//     TEST_IGNORE_MESSAGE("Need to Implement CAN_messages");
// }

// #define RANDOM_BLEH
#ifdef RANDOM_BLEH
void test_randomStuff()
{
    uint8_t array1[3] = {1, 2, 3};
    uint8_t array2[3] = {1, 2, 3};
    uint8_t array3[3] = {1, 4, 4};
                           // 18446744073709551615"
    // uint64_t longlong = 18446744073709551614;
    // int_least64_t longlong = -18446744073709551614;
    // unsigned long long int thelongone = ULLONG_MAX;
    unsigned long long int thelongone = ULLONG_MAX;
    thelongone = 18446744073709551615ULL;
    printf("%lli\r\n", LLONG_MAX);
    printf("%lli\r\n", LLONG_MIN);
    printf("%llu\r\n", ULLONG_MAX);
    printf("%llu\r\n", thelongone);
    thelongone = (0b1 << 29);
    printf("%llu\r\n", thelongone);
    thelongone = (uint64_t)0b1ULL << 55;
    printf("%llu\r\n", thelongone);
    thelongone = (1ULL << 63);
    printf("%llu\r\n", thelongone);
    thelongone = (ULLONG_MAX >> 63);
    printf("%llu\r\n", thelongone);
    // printf("%llu\r\n", LLONG_MIN);
    hello_world(1);
    for(int i; i < 10; i++)
    {
        printf("i is %i\r", i);
    }

    TEST_ASSERT_EQUAL_UINT8_ARRAY(array1, array2, 3);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(array1, array3, 3);


}
#endif

#endif // TEST
