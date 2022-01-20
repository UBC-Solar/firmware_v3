#ifdef TEST

#include "unity.h"
#include "soc.h"
#include "mock_ltc6813_btm.h"


void setUp(void)
{

}

void tearDown(void)
{

}

void test_indexOfNearestCellVoltage()
{
    printf("test_indexOfNearestCellVoltage\n");
    
    float cell_voltage = 4.38;
    int expected_index = 2; //4.40V
    
    int calculated_index = indexOfNearestCellVoltage(cell_voltage);
    // int calculated_index = indexOfNearestCellVoltage2(cell_voltage); //this can make Ceedling run

    printf("index expected: %d\n", expected_index);
    printf("index found: %d\n", calculated_index);

    TEST_ASSERT_EQUAL(expected_index, calculated_index);
}






#endif // TEST