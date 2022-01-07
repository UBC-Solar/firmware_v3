#ifdef TEST

#include "unity.h"

#include "soc.h"
#include "soc.c"
#include "mock_ltc6813_btm.h"
// #include "mock_ltc6813_btm_bal.h"
// #include "mock_stm32f1xx_hal.h"
// #include "mock_stm32f1xx_hal_gpio.h"

// useful debug link
// http://www.electronvector.com/blog/add-unit-tests-to-your-current-project-with-ceedling


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

//------------------------------------

//Ib = 0

// void test_soc_IDLE(void)
// {
//     SOC_init(); //set all SOC previousCurrent, previousTime, and DoDTotal to 0 V

//     uint32_t cell_voltage = 3.0 * 10000; // 3.0 * 100uV
//     int32_t current_from_CAN = 0; //1.0 A
//     uint32_t time_duration = 1000; //1.0 s
    
//     TEST_ASSERT_EQUAL_DOUBLE_WITHIN(0.1, 100, (double) stateOfChargeWithCurrent(cell_voltage, current_from_CAN, time_duration));
// }


// //Ib > 0 --> battery charging

// //fully charged

// void test_soc_FullyCharged(void)
// {
//     SOC_init(); //set all SOC previousCurrent, previousTime, and DoDTotal to 0

//     uint32_t cell_voltage = 4.2 * 10000; // 4.2 * 100uV
//     int32_t current_from_CAN = 1; //1.0 A
//     uint32_t time_duration = 1000; //1.0 s
    
//     TEST_ASSERT_EQUAL_DOUBLE(100.0, (double) stateOfChargeWithCurrent(cell_voltage, current_from_CAN, time_duration));
// }

// //not fully charged

// void test_soc_Charging(void)
// {
//     SOC_init(); //set all SOC previousCurrent, previousTime, and DoDTotal to 0

//     uint32_t cell_voltage = 3.0 * 10000; // * 100uV
//     int32_t current_from_CAN = 10; // A
//     uint32_t time_duration = 30 * 1000; //ms
    
//     TEST_ASSERT_EQUAL_DOUBLE_WITHIN(0.1, 110.12, (double) stateOfChargeWithCurrent(cell_voltage, current_from_CAN, time_duration));
// }

// //Ib < 0 --> battery discharging

// //fully discharged

// void test_soc_EmptyCharge(void)
// {
//     SOC_init(); //set all SOC previousCurrent, previousTime, and DoDTotal to 0

//     uint32_t cell_voltage = 2.5 * 10000; // 2.5 * 100uV
//     int32_t current_from_CAN = -1; //1.0 A
//     uint32_t time_duration = 1000; //1.0 s
    
//     TEST_ASSERT_EQUAL_DOUBLE(0.0, (double) stateOfChargeWithCurrent(cell_voltage, current_from_CAN, time_duration));
// }

// //not fully discharged

// void test_soc_Discharging(void)
// {
//     SOC_init(); //set all SOC previousCurrent, previousTime, and DoDTotal to 0

//     uint32_t cell_voltage = 3.0 * 10000; // 4.2 * 100uV
//     int32_t current_from_CAN = -10; //A
//     uint32_t time_duration = 30 * 1000; //ms
    
//     TEST_ASSERT_EQUAL_DOUBLE_WITHIN(0.1, 98.988, (double) stateOfChargeWithCurrent(cell_voltage, current_from_CAN, time_duration));
// }







#endif // TEST