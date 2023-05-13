#ifdef TEST

// Standard unit test include
#include "unity.h"

// Code module under test
#include "soc.h"

// Mocked code modules
#include "mock_stm32f1xx_hal.h"
#include "mock_ltc6813_btm.h"


void setUp(void)
{

}

void tearDown(void)
{

}

// indexOfNearestCellVoltage(cell_voltage) function

void test_indexOfNearestCellVoltage1()
{
    printf("test_indexOfNearestCellVoltage1\n");
    
    float cell_voltage = 3.68;
    int expected_index = 16; //3.70V
    
    int calculated_index = indexOfNearestCellVoltage(cell_voltage);

    printf("index expected: %d\n", expected_index);
    printf("index found: %d\n", calculated_index);

    TEST_ASSERT_EQUAL(expected_index, calculated_index);
}

void test_indexOfNearestCellVoltage2()
{
    printf("test_indexOfNearestCellVoltage2\n");
    
    float cell_voltage = 3.225;
    int expected_index = 26; //3.20V
    
    int calculated_index = indexOfNearestCellVoltage(cell_voltage);

    printf("index expected: %d\n", expected_index);
    printf("index found: %d\n", calculated_index);

    TEST_ASSERT_EQUAL(expected_index, calculated_index);
}

void test_indexOfNearestCellVoltage3()
{
    printf("test_indexOfNearestCellVoltage3\n");
    
    float cell_voltage = 3.228;
    int expected_index = 25; //3.25V
    
    int calculated_index = indexOfNearestCellVoltage(cell_voltage);

    printf("index expected: %d\n", expected_index);
    printf("index found: %d\n", calculated_index);

    TEST_ASSERT_EQUAL(expected_index, calculated_index);
}

void test_indexOfNearestCellVoltageOvercharged()
{
    printf("test_indexOfNearestCellVoltageOvercharged\n");
    
    float cell_voltage = 5.00;
    int expected_index = 0; //4.50V
    
    int calculated_index = indexOfNearestCellVoltage(cell_voltage);

    printf("index expected: %d\n", expected_index);
    printf("index found: %d\n", calculated_index);

    TEST_ASSERT_EQUAL(expected_index, calculated_index);
}

void test_indexOfNearestCellVoltageOverDischarged()
{
    printf("test_indexOfNearestCellVoltageOverDischarged\n");
    
    float cell_voltage = 1.00;
    int expected_index = 40; //2.50V
    
    int calculated_index = indexOfNearestCellVoltage(cell_voltage);

    printf("index expected: %d\n", expected_index);
    printf("index found: %d\n", calculated_index);

    TEST_ASSERT_EQUAL(expected_index, calculated_index);
}


//SOC_moduleInit(float cell_voltage)

void test_SOC_moduleInitWithinCurveLessThanFirstPoint()
{
    printf("test_SOC_moduleInitWithinCurveLessThanFirstPoint\n");

    float cell_voltage = 3.38;
    float expected_SOC = 39.5942029;
    float tolerance = 0.01;

    float calculated_SOC = SOC_moduleInit(cell_voltage);

    printf("Module SOC expected: %f\n", expected_SOC);
    printf("Module SOC found: %f\n", calculated_SOC);

    TEST_ASSERT_FLOAT_WITHIN(tolerance, expected_SOC, calculated_SOC);
}

void test_SOC_moduleInitWithinCurveGreaterThanFirstPoint()
{
    printf("test_SOC_moduleInitWithinCurveGreaterThanFirstPoint\n");

    float cell_voltage = 3.46;
    float expected_SOC = 48.52173913;
    float tolerance = 0.01;

    float calculated_SOC = SOC_moduleInit(cell_voltage);

    printf("Module SOC expected: %f\n", expected_SOC);
    printf("Module SOC found: %f\n", calculated_SOC);

    TEST_ASSERT_FLOAT_WITHIN(tolerance, expected_SOC, calculated_SOC);
}

void test_SOC_moduleInitOvercharged()
{
    printf("test_SOC_moduleInitOvercharged\n");

    float cell_voltage = 6.30;
    float expected_SOC = 100.0;
    float tolerance = 0.01;

    float calculated_SOC = SOC_moduleInit(cell_voltage);

    printf("Module SOC expected: %f\n", expected_SOC);
    printf("Module SOC found: %f\n", calculated_SOC);

    TEST_ASSERT_FLOAT_WITHIN(tolerance, expected_SOC, calculated_SOC);
}

void test_SOC_moduleInitOverDischarged()
{
    printf("test_SOC_moduleInitOvercharged\n");

    float cell_voltage = 1.234;
    float expected_SOC = -3.28115942;
    float tolerance = 0.01;

    float calculated_SOC = SOC_moduleInit(cell_voltage);

    printf("Module SOC expected: %f\n", expected_SOC);
    printf("Module SOC found: %f\n", calculated_SOC);

    TEST_ASSERT_FLOAT_WITHIN(tolerance, expected_SOC, calculated_SOC);
}

void test_SOC_moduleInitOverDischargedNegativeVoltage()
{
    printf("test_SOC_moduleInitOverDischargedNegativeVoltage\n");

    float cell_voltage = -0.658;
    float expected_SOC = -14.24927536;
    float tolerance = 0.01;

    float calculated_SOC = SOC_moduleInit(cell_voltage);

    printf("Module SOC expected: %f\n", expected_SOC);
    printf("Module SOC found: %f\n", calculated_SOC);

    TEST_ASSERT_FLOAT_WITHIN(tolerance, expected_SOC, calculated_SOC);
}

void test_SOC_moduleInitEqualCellVoltage()
{
    printf("test_SOC_moduleInitOverDischargedNegativeVoltage\n");

    float cell_voltage = 3.35;
    float expected_SOC = 35.942;
    float tolerance = 0.01;

    float calculated_SOC = SOC_moduleInit(cell_voltage);

    printf("Module SOC expected: %f\n", expected_SOC);
    printf("Module SOC found: %f\n", calculated_SOC);

    TEST_ASSERT_FLOAT_WITHIN(tolerance, expected_SOC, calculated_SOC);
}

void test_calculateDeltaDODPositiveCurrents()
{
    printf("test_calculateDeltaDODPositiveCurrents\n");

    float present_time = 4351.2; //ms
    float past_time = 2225.13; //ms
    float present_current = 4.55; //A
    float past_current = 2.30; //A

    float expected_delta_DOD = -16.23587458;
    float tolerance = 0.01;

    float calculated_delta_DOD = calculateDeltaDOD(present_current, present_time, past_current, past_time);

    printf("Module Change In DOD expected: %f\n", expected_delta_DOD);
    printf("Module Change In DOD found: %f\n", calculated_delta_DOD);

    TEST_ASSERT_FLOAT_WITHIN(tolerance, expected_delta_DOD, calculated_delta_DOD);
}

void test_calculateDeltaDODNegativeCurrents()
{
    printf("test_calculateDeltaDODNegativeCurrents\n");

    float present_time = 4351.2; //ms
    float past_time = 2225.13; //ms
    float present_current = -2.523; //A
    float past_current = -2.6517; //A

    float expected_delta_DOD = 12.2650774;
    float tolerance = 0.01;

    float calculated_delta_DOD = calculateDeltaDOD(present_current, present_time, past_current, past_time);

    printf("Module Change In DOD expected: %f\n", expected_delta_DOD);
    printf("Module Change In DOD found: %f\n", calculated_delta_DOD);

    TEST_ASSERT_FLOAT_WITHIN(tolerance, expected_delta_DOD, calculated_delta_DOD);
}

void test_calculateDeltaDODNegativePositiveCurrents()
{
    printf("test_calculateDeltaDODNegativePositiveCurrents\n");

    float present_time = 4351.2; //ms
    float past_time = 2225.13; //ms
    float present_current = -1.678; //A
    float past_current = 2.372; //A

    float expected_delta_DOD = -1.644919264;
    float tolerance = 0.01;

    float calculated_delta_DOD = calculateDeltaDOD(present_current, present_time, past_current, past_time);

    printf("Module Change In DOD expected: %f\n", expected_delta_DOD);
    printf("Module Change In DOD found: %f\n", calculated_delta_DOD);

    TEST_ASSERT_FLOAT_WITHIN(tolerance, expected_delta_DOD, calculated_delta_DOD);
}

void test_calculateDeltaDODPositiveNegativeCurrents()
{
    printf("test_calculateDeltaDODPositiveNegativeCurrents\n");

    float present_time = 4351.2; //ms
    float past_time = 2225.13; //ms
    float present_current = 10.7452; //A
    float past_current = -0.698; //A

    float expected_delta_DOD = -23.81388016;
    float tolerance = 0.01;

    float calculated_delta_DOD = calculateDeltaDOD(present_current, present_time, past_current, past_time);

    printf("Module Change In DOD expected: %f\n", expected_delta_DOD);
    printf("Module Change In DOD found: %f\n", calculated_delta_DOD);

    TEST_ASSERT_FLOAT_WITHIN(tolerance, expected_delta_DOD, calculated_delta_DOD);
}

void test_SOC_moduleEstZeroNetCurrent()
{
    printf("test_SOC_moduleEstZeroNetCurrent\n");

    float total_time = 4345.123; //ms
    float cell_voltage = 3.5 / 0.0001; //100 uV
    float last_SOC = 52.46376812; //%
    float current_reading = 0.0; //A

    float expected_SOC = 52.46376812;
    float tolerance = 0.01;

    float calculated_SOC = SOC_moduleEst(last_SOC, cell_voltage, current_reading, total_time);

    printf("Module SOC expected: %f\n", expected_SOC);
    printf("Module SOC found: %f\n", calculated_SOC);

    TEST_ASSERT_FLOAT_WITHIN(tolerance, expected_SOC, calculated_SOC);
}

void test_SOC_moduleEstPositiveNetCurrent()
{
    printf("test_SOC_moduleEstPositiveNetCurrent\n");

    uint32_t present_time = 4345; //ms
    float last_SOC = 52.46376812; //% at 3.5V
    int32_t current_reading = 2; //A
    uint32_t cell_voltage = 36340; //100 uV

    float expected_SOC = 60.69843924;
    float tolerance = 0.01;

    float calculated_SOC = SOC_moduleEst(last_SOC, cell_voltage, current_reading, present_time);

    printf("Module SOC expected: %f\n", expected_SOC);
    printf("Module SOC found: %f\n", calculated_SOC);

    TEST_ASSERT_FLOAT_WITHIN(tolerance, expected_SOC, calculated_SOC);
}

void test_SOC_moduleEstPositiveNetCurrentOvercharged()
{
    printf("test_SOC_moduleEstPositiveNetCurrentOvercharged\n");

    uint32_t present_time = 4345; //ms
    float last_SOC = 52.46376812; //% at 3.5V
    int32_t current_reading = 2; //A
    uint32_t cell_voltage = 45000; //100 uV

    float expected_SOC = 100.0;
    float tolerance = 0.01;

    float calculated_SOC = SOC_moduleEst(last_SOC, cell_voltage, current_reading, present_time);

    printf("Module SOC expected: %f\n", expected_SOC);
    printf("Module SOC found: %f\n", calculated_SOC);

    TEST_ASSERT_FLOAT_WITHIN(tolerance, expected_SOC, calculated_SOC);
}

void test_SOC_moduleEstNegativeNetCurrent()
{
    printf("test_SOC_moduleEstNegativeNetCurrent\n");

    uint32_t present_time = 4345; //ms
    float last_SOC = 52.46376812; //% at 3.5V
    int32_t current_reading = -2; //A
    uint32_t cell_voltage = 30000; //100 uV

    float expected_SOC = 44.22909699;
    float tolerance = 0.01;

    float calculated_SOC = SOC_moduleEst(last_SOC, cell_voltage, current_reading, present_time);

    printf("Module SOC expected: %f\n", expected_SOC);
    printf("Module SOC found: %f\n", calculated_SOC);

    TEST_ASSERT_FLOAT_WITHIN(tolerance, expected_SOC, calculated_SOC);
}

void test_SOC_moduleEstNegativeNetCurrentOverdischarged()
{
    printf("test_SOC_moduleEstNegativeNetCurrentOverdischarged\n");

    uint32_t present_time = 4345; //ms
    float last_SOC = 52.46376812; //% at 3.5V
    int32_t current_reading = -2; //A
    uint32_t cell_voltage = 20000; //100 uV

    float expected_SOC = 1.15942029;
    float tolerance = 0.01;

    float calculated_SOC = SOC_moduleEst(last_SOC, cell_voltage, current_reading, present_time);

    printf("Module SOC expected: %f\n", expected_SOC);
    printf("Module SOC found: %f\n", calculated_SOC);

    TEST_ASSERT_FLOAT_WITHIN(tolerance, expected_SOC, calculated_SOC);
}


#endif // TEST
