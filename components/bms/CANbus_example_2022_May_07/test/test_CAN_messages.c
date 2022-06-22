#ifdef TEST

// #define ENABLE_PRINTF_MESSAGES

#include "unity.h"
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






void setUp(void)
{
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
    debugStrings_faults[0]  = "BMS fault bit";
    debugStrings_faults[29] = "HLIM is set: cannot charge";
    debugStrings_faults[30] = "LLIM is set: cannot discharge";
    debugStrings_faults[41] = "Interlock is tripped";
    debugStrings_faults[42] = "Communication fault with cell";
    debugStrings_faults[43] = "Charge overcurrent";
    debugStrings_faults[44] = "Discharge overcurrent";
    debugStrings_faults[45] = "Over temperature";
    debugStrings_faults[46] = "Under voltage";
    debugStrings_faults[47] = "Over voltage";
    debugStrings_faults[48] = "Low voltage warning";
    debugStrings_faults[49] = "High voltage warning";
    debugStrings_faults[50] = "Charge overcurrent warning";
    debugStrings_faults[51] = "Discharge overcurrent warning";
    debugStrings_faults[52] = "Cold temperature warning";
    debugStrings_faults[53] = "Hot temperature warning";
    debugStrings_faults[54] = "Low SOH warning";
    debugStrings_faults[55] = "Isolation fault warning";



    //testing all fault flags
    CAN_createExpectedMessage622withFaultFlag(0b11111111);

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
            printf("%s \r\n", debugStrings_faults[i]);
            expectedMessage = CAN_createExpectedMessage622withBitFlag(i);
            actualMessage   = CAN_createIdealMessage624withBitFlag(i);

            //check size of messages
            expectedSize = sizeof(expectedMessage)/sizeof(expectedMessage[0]);
            actualSize   = sizeof(actualMessage)  /sizeof(actualMessage[0]);

            TEST_ASSERT_EQUAL(expectedSize, actualSize);

            //compare message contents
            TEST_ASSERT_EQUAL(expectedMessage[0], actualMessage[0]);
            TEST_ASSERT_EQUAL(expectedMessage[1], actualMessage[1]);
            TEST_ASSERT_EQUAL(expectedMessage[2], actualMessage[2]);
            TEST_ASSERT_EQUAL(expectedMessage[3], actualMessage[3]);
            TEST_ASSERT_EQUAL(expectedMessage[4], actualMessage[4]);
            TEST_ASSERT_EQUAL(expectedMessage[5], actualMessage[5]);
            TEST_ASSERT_EQUAL(expectedMessage[6], actualMessage[6]);
        }
    }

    //testing time field
    CAN_createExpectedMessage622withTime(0b1111000011110000);

    //test min time, min + 100 time, and max time
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
void test_CANstate_staleCheck()
{
        uint8_t expectValue = CAN_STALE;
        HAL_CAN_GetTxMailboxesFreeLevel_IgnoreAndReturn(expectValue);
        uint8_t actualValue = CANstate_staleCheck();

        TEST_ASSERT_EQUAL_UINT8(expectValue,actualValue);
}

// void test_CAN_messages_NeedToImplement(void)
// {
//     TEST_IGNORE_MESSAGE("Need to Implement CAN_messages");
// }



void test_randomStuff()
{
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
}

#endif // TEST
