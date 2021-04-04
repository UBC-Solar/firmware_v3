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













void test1(){

 do {if ((

1

)) {} else {UnityFail( ((" Expression Evaluated To FALSE")), (UNITY_UINT)((UNITY_UINT)(25)));}} while(0);

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

), (UNITY_UINT)(37));

}



void test_checkLTCtemp(){

 BTM_Status_t expected_status = {BTM_ERROR_TIMEOUT, 0};

 BTM_sendCmdAndPoll_CMockExpectAndReturn(42, CMD_ADSTAT_ITMP, expected_status);



 BTM_Status_t outStatus = ST_checkLTCtemp();

 UnityAssertEqualNumber((UNITY_INT)((expected_status.error)), (UNITY_INT)((outStatus.error)), (

((void *)0)

), (UNITY_UINT)(45), UNITY_DISPLAY_STYLE_INT);

 UnityAssertEqualNumber((UNITY_INT)((expected_status.device_num)), (UNITY_INT)((outStatus.device_num)), (

((void *)0)

), (UNITY_UINT)(46), UNITY_DISPLAY_STYLE_INT);

}
