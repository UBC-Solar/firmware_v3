#ifdef TEST

#include "unity.h"

#include "selftest.c"
#include "selftest.h"
#include "mock_ltc6813_btm.h"
#include "mock_ltc6813_btm_bal.h"

void setUp(void)
{

}

void tearDown(void)
{
}

/*void test_selftest_NeedToImplement(void)
{
    TEST_IGNORE_MESSAGE("Need to Implement selftest");
}*/

void test1(){
	TEST_ASSERT(true);
}


void test_itmpConversion(){
	uint16_t itmp[] = {0x595C};
	float expected_temp = 25.0f;
	float temp_celsius[1];
	float tolerance = 1.0f;
	itmpConversion(itmp, temp_celsius);

	printf("converted temp: %f", temp_celsius[0]);
	TEST_ASSERT_FLOAT_WITHIN(tolerance, expected_temp, temp_celsius[0]);
}

void test_checkLTCtemp(){
	BTM_Status_t expected_status = {BTM_ERROR_TIMEOUT, 0};
	BTM_sendCmdAndPoll_ExpectAndReturn(CMD_ADSTAT_ITMP, expected_status);

	BTM_Status_t outStatus = ST_checkLTCtemp();
	TEST_ASSERT_EQUAL(expected_status.error, outStatus.error);
	TEST_ASSERT_EQUAL(expected_status.device_num, outStatus.device_num);
}

#endif // TEST
