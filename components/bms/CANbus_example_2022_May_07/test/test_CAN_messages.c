#ifdef TEST

// #define ENABLE_PRINTF_MESSAGES

#include "unity.h"
#include <stdlib.h>
#include <stdint.h>

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

#endif // TEST
