#include "Core/Src/selftest.c"
#include "build/test/mocks/mock_ltc6813_btm_bal.h"
#include "build/test/mocks/mock_ltc6813_btm.h"
#include "Core/Src/selftest.h"
#include "C:/Ruby26-x64/lib/ruby/gems/2.6.0/gems/ceedling-0.31.0/vendor/unity/src/unity.h"








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

 UnityAssertFloatsWithin((UNITY_FLOAT)((tolerance)), (UNITY_FLOAT)((expected_temp)), (UNITY_FLOAT)((temp_celsius[0])), (

((void *)0)

), (UNITY_UINT)(32));

}



void test_checkLTCtemp1(){

 BTM_Status_t expected_status = {BTM_ERROR_TIMEOUT, 0};

 BTM_sendCmdAndPoll_CMockExpectAndReturn(37, CMD_ADSTAT_ITMP, expected_status);



 BTM_Status_t outStatus = ST_checkLTCtemp();

 UnityAssertEqualNumber((UNITY_INT)((expected_status.error)), (UNITY_INT)((outStatus.error)), (

((void *)0)

), (UNITY_UINT)(40), UNITY_DISPLAY_STYLE_INT);

 UnityAssertEqualNumber((UNITY_INT)((expected_status.device_num)), (UNITY_INT)((outStatus.device_num)), (

((void *)0)

), (UNITY_UINT)(41), UNITY_DISPLAY_STYLE_INT);

}



void test_checkLTCtemp2(){

 BTM_Status_t sendCmdAndPoll_status = {BTM_OK, 0};

 BTM_sendCmdAndPoll_CMockExpectAndReturn(46, CMD_ADSTAT_ITMP, sendCmdAndPoll_status);



 BTM_Status_t readRegisterGroup_status = {BTM_ERROR_HAL, 1};

 BTM_readRegisterGroup_CMockIgnoreAndReturn(49, readRegisterGroup_status);



 BTM_Status_t outStatus = ST_checkLTCtemp();

 UnityAssertEqualNumber((UNITY_INT)((readRegisterGroup_status.error)), (UNITY_INT)((outStatus.error)), (

((void *)0)

), (UNITY_UINT)(52), UNITY_DISPLAY_STYLE_INT);

 UnityAssertEqualNumber((UNITY_INT)((readRegisterGroup_status.device_num)), (UNITY_INT)((outStatus.device_num)), (

((void *)0)

), (UNITY_UINT)(53), UNITY_DISPLAY_STYLE_INT);

}



void test_checkLTCtemp3(){

 BTM_Status_t sendCmdAndPoll_status = {BTM_OK, 0};

 BTM_sendCmdAndPoll_CMockExpectAndReturn(58, CMD_ADSTAT_ITMP, sendCmdAndPoll_status);



 BTM_Status_t readRegisterGroup_status = {BTM_ERROR_HAL, 1};

 uint8_t rx_data[2U][6];

 uint8_t (*ptr_rx_data)[6] = rx_data;

 BTM_readRegisterGroup_CMockExpectAndReturn(63, CMD_RDSTATA, 

((void *)0)

, readRegisterGroup_status);





 BTM_Status_t outStatus = ST_checkLTCtemp();

 UnityAssertEqualNumber((UNITY_INT)((readRegisterGroup_status.error)), (UNITY_INT)((outStatus.error)), (

((void *)0)

), (UNITY_UINT)(67), UNITY_DISPLAY_STYLE_INT);

 UnityAssertEqualNumber((UNITY_INT)((readRegisterGroup_status.device_num)), (UNITY_INT)((outStatus.device_num)), (

((void *)0)

), (UNITY_UINT)(68), UNITY_DISPLAY_STYLE_INT);

}



void test_shiftDchStatus0(){

 BTM_module_bal_status_t dch_off = {DISCHARGE_OFF};

    BTM_module_bal_status_t dch_on = {DISCHARGE_ON};

 BTM_module_bal_status_t dch_set[18] = {dch_on, dch_off, dch_off,

                                                        dch_off, dch_off, dch_off,

                                                        dch_on, dch_off, dch_off,

                                                        dch_off, dch_off, dch_off,

                                                        dch_on, dch_off, dch_off,

                                                        dch_off, dch_off, dch_off};

 BTM_module_bal_status_t dch_out[18] = {dch_off, dch_on, dch_off,

                                                        dch_off, dch_off, dch_off,

                                                        dch_off, dch_on, dch_off,

                                                        dch_off, dch_off, dch_off,

                                                        dch_off, dch_on, dch_off,

                                                        dch_off, dch_off, dch_off};

 shiftDchStatus(dch_set);

 for (int i = 0; i < 18; i++){

  UnityAssertEqualNumber((UNITY_INT)((dch_out[i])), (UNITY_INT)((dch_set[i])), (

 ((void *)0)

 ), (UNITY_UINT)(88), UNITY_DISPLAY_STYLE_INT);

 }

}



void test_shiftDchStatus1(){

 BTM_module_bal_status_t dch_off = {DISCHARGE_OFF};

    BTM_module_bal_status_t dch_on = {DISCHARGE_ON};

 BTM_module_bal_status_t dch_set[18] = {dch_on, dch_off, dch_off,

                                                        dch_off, dch_off, dch_off,

                                                        dch_on, dch_off, dch_off,

                                                        dch_off, dch_off, dch_off,

                                                        dch_on, dch_off, dch_off,

                                                        dch_off, dch_off, dch_off};

 BTM_module_bal_status_t dch_out[18] = {dch_on, dch_off, dch_off,

                                                        dch_off, dch_off, dch_off,

                                                        dch_on, dch_off, dch_off,

                                                        dch_off, dch_off, dch_off,

                                                        dch_on, dch_off, dch_off,

                                                        dch_off, dch_off, dch_off};

 for (int i = 0; i < 18; i++){

  shiftDchStatus(dch_set);

 }

 for (int i = 0; i < 18; i++){

  UnityAssertEqualNumber((UNITY_INT)((dch_out[i])), (UNITY_INT)((dch_set[i])), (

 ((void *)0)

 ), (UNITY_UINT)(111), UNITY_DISPLAY_STYLE_INT);

 }

}
