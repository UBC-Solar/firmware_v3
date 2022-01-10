#ifdef TEST

#include "unity.h"

#include "selftest.h"

#include "mock_ltc6813_btm.h"
#include "mock_ltc6813_btm_bal.h"

void setUp(void)
{

}

void tearDown(void)
{

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

void test_checkLTCtemp1(){
	BTM_Status_t expected_status = {BTM_ERROR_TIMEOUT, 0};
	BTM_sendCmdAndPoll_ExpectAndReturn(CMD_ADSTAT_ITMP, expected_status);

	BTM_Status_t outStatus = ST_checkLTCtemp();
	TEST_ASSERT_EQUAL(expected_status.error, outStatus.error);
	TEST_ASSERT_EQUAL(expected_status.device_num, outStatus.device_num);
}

void test_checkLTCtemp2(){
	BTM_Status_t sendCmdAndPoll_status = {BTM_OK, 0};
	BTM_sendCmdAndPoll_ExpectAndReturn(CMD_ADSTAT_ITMP, sendCmdAndPoll_status);

	BTM_Status_t readRegisterGroup_status = {BTM_ERROR_HAL, 1};
	BTM_readRegisterGroup_IgnoreAndReturn(readRegisterGroup_status);

	BTM_Status_t outStatus = ST_checkLTCtemp();
	TEST_ASSERT_EQUAL(readRegisterGroup_status.error, outStatus.error);
	TEST_ASSERT_EQUAL(readRegisterGroup_status.device_num, outStatus.device_num);
}

void test_shiftDchStatus0(){
	BTM_module_bal_status_t dch_off = {DISCHARGE_OFF};
    BTM_module_bal_status_t dch_on = {DISCHARGE_ON};
	BTM_module_bal_status_t dch_set[BTM_NUM_MODULES] = {dch_on, dch_off, dch_off,
                                                        dch_off, dch_off, dch_off,
                                                        dch_on, dch_off, dch_off,
                                                        dch_off, dch_off, dch_off,
                                                        dch_on, dch_off, dch_off,
                                                        dch_off, dch_off, dch_off};
	BTM_module_bal_status_t dch_out[BTM_NUM_MODULES] = {dch_off, dch_on, dch_off,
                                                        dch_off, dch_off, dch_off,
                                                        dch_off, dch_on, dch_off,
                                                        dch_off, dch_off, dch_off,
                                                        dch_off, dch_on, dch_off,
                                                        dch_off, dch_off, dch_off};
	shiftDchStatus(dch_set);
	for (int i = 0; i < BTM_NUM_MODULES; i++){
		TEST_ASSERT_EQUAL(dch_out[i], dch_set[i]);
	}
}

void test_shiftDchStatus1(){
	BTM_module_bal_status_t dch_off = {DISCHARGE_OFF};
    BTM_module_bal_status_t dch_on = {DISCHARGE_ON};
	BTM_module_bal_status_t dch_set[BTM_NUM_MODULES] = {dch_on, dch_off, dch_off,
                                                        dch_off, dch_off, dch_off,
                                                        dch_on, dch_off, dch_off,
                                                        dch_off, dch_off, dch_off,
                                                        dch_on, dch_off, dch_off,
                                                        dch_off, dch_off, dch_off};
	BTM_module_bal_status_t dch_out[BTM_NUM_MODULES] = {dch_on, dch_off, dch_off,
                                                        dch_off, dch_off, dch_off,
                                                        dch_on, dch_off, dch_off,
                                                        dch_off, dch_off, dch_off,
                                                        dch_on, dch_off, dch_off,
                                                        dch_off, dch_off, dch_off};
	for (int i = 0; i < BTM_NUM_MODULES; i++){
		shiftDchStatus(dch_set);
	}
	for (int i = 0; i < BTM_NUM_MODULES; i++){
		TEST_ASSERT_EQUAL(dch_out[i], dch_set[i]);
	}
}

#endif // TEST
