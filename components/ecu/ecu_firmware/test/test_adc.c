#ifdef TEST

#include <unity.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "adc.h"

//#include "mock_stm32f1xx_hal.h"

float ** enumerateGetSetValsOverRange(int adc_channel, int (*get_function_int)(), float (*get_function_float)(), float start_point[], float end_point[], 
                    int num_samples, float adc_offset, double adc_scale_factor);

uint32_t* generateSineCurve(int num_points, int num_cycles, float amplitude, float y_offset);


void setUp(void)
{

}

void tearDown(void)
{

}

void test_ADC_getAndSetOffsetRef_AM()
{

    int num_samples = 20;
    float start_point[] = {-4.0, -1400.0};
    float end_point[] = {15.0, 5250.0};

    printf("\nTesting AM offset reference...\n     Input:   expected||    actual\n");
    float** test_results = enumerateGetSetValsOverRange(OFFSET_REF_AM__ADC1_IN0, NULL, ADC_getOffsetRef_AM, start_point, end_point,
                                                        num_samples, 0.0, 1000.0);
    for (int n = 0; n < num_samples; n++) TEST_ASSERT_FLOAT_WITHIN(1.0f, test_results[0][n], test_results[1][n]);
}


void test_ADC_getAndSetOffsetRef_Batt()
{

    int num_samples = 20;
    float start_point[] = {-4.0, -1400.0};
    float end_point[] = {15.0, 5250.0};

    printf("\nTesting battery offset reference...\n     Input:   expected||    actual\n");
    float** test_results = enumerateGetSetValsOverRange(OFFSET_REF_BAT__ADC1_IN10, NULL, ADC_getOffsetRef_Batt, start_point, end_point,
                                                        num_samples, 0.0, 1000.0);
    for (int n = 0; n < num_samples; n++) TEST_ASSERT_FLOAT_WITHIN(1.0f, test_results[0][n], test_results[1][n]);
}


void test_ADC_getAndSetLowVoltageCurrent()
{

    int num_samples = 20;
    float start_point[] = {-4.0, -1400.0};
    float end_point[] = {15.0, 5250.0};

    printf("\nTesting low voltage current...\n     Input:   expected||    actual\n");
    float** test_results = enumerateGetSetValsOverRange(LVS_CURR_SENSE__ADC1_IN4, ADC_getLowVoltageCurrent, NULL, start_point, end_point,
                                                        num_samples, -3.3/2, (1.0/26.4)*100.0*1000.0);
    for (int n = 0; n < num_samples; n++) TEST_ASSERT_FLOAT_WITHIN(1.0f, test_results[0][n], test_results[1][n]);
}   

void test_ADC_getAndSetSuppBatteryVoltage()
{
    int num_samples = 20;
    float start_point[] = {-4.0, -1400.0};
    float end_point[] = {15.0, 5250.0};

    printf("\nTesting supplimentary battery voltage...\n     Input:   expected||    actual\n");
    float** test_results = enumerateGetSetValsOverRange(SUPP_SENSE__ADC1_IN5, ADC_getSuppBattVoltage, NULL, start_point, end_point,
                                                        num_samples, 0.0, 11.0*1000);
    for (int n = 0; n < num_samples; n++) TEST_ASSERT_FLOAT_WITHIN(2.0f, test_results[0][n], test_results[1][n]);
}

void test_ADC_getAndSetBatteryCurrent()
{
    int num_samples = 20;
    float start_point[] = {-4.0, -1400.0};
    float end_point[] = {15.0, 5250.0};
    float offset_adc_val = 1861.0;

    printf("\nTesting main battery current...\n     Input:   expected||    actual\n");
    ADC_setReading(offset_adc_val, OFFSET_REF_BAT__ADC1_IN10);
    float** test_results = enumerateGetSetValsOverRange(B_SENSE__ADC3_IN1, ADC_getBatteryCurrent, NULL, start_point, end_point,
                                                        num_samples, -3.3*offset_adc_val/4095.0, 40.0*100.0);
    for (int n = 0; n < num_samples; n++) TEST_ASSERT_FLOAT_WITHIN(4.0f, test_results[0][n], test_results[1][n]);
}

void test_ADC_getAndSetMotorCurrent()
{
    int num_samples = 20;
    float start_point[] = {-4.0, -1400.0};
    float end_point[] = {15.0, 5250.0};
    float offset_adc_val = 1861.0;

    printf("\nTesting motor current...\n     Input:   expected||    actual\n");
    ADC_setReading(offset_adc_val, OFFSET_REF_AM__ADC1_IN0);
    float** test_results = enumerateGetSetValsOverRange(M_SENSE__ADC3_IN2, ADC_getMotorCurrent, NULL, start_point, end_point,
                                                        num_samples, -3.3*offset_adc_val/4095.0, 80.0*100.0);
    for (int n = 0; n < num_samples; n++) TEST_ASSERT_FLOAT_WITHIN(7.0f, test_results[0][n], test_results[1][n]);
}

void test_ADC_getAndSetArrayCurrent(){
    int num_samples = 20;
    float start_point[] = {-4.0, -1400.0};
    float end_point[] = {15.0, 5250.0};
    float offset_adc_val = 1861.0;

    printf("\nTesting array current...\n     Input:   expected||    actual\n");
    ADC_setReading(offset_adc_val, OFFSET_REF_AM__ADC1_IN0);
    float** test_results = enumerateGetSetValsOverRange(A_SENSE__ADC3_IN3, ADC_getArrayCurrent, NULL, start_point, end_point,
                                                        num_samples, -3.3*offset_adc_val/4095.0, 80.0*100.0);
    for (int n = 0; n < num_samples; n++) TEST_ASSERT_FLOAT_WITHIN(7.0f, test_results[0][n], test_results[1][n]);
}

void test_ADC_netCurrentOut(){
    
    int adc_resolution = 4096-1;

    float motor_adc_reading = 3250.0;
    float array_adc_reading = 3150.0;

    float offset_adc_val = 1861.0;

    float motor_expected_I = ((motor_adc_reading*3.3/adc_resolution)-(offset_adc_val*3.3/adc_resolution))*100*80;
    float array_expected_I = ((array_adc_reading*3.3/adc_resolution)-(offset_adc_val*3.3/adc_resolution))*100*80;

    //ADC_setMotorCurrent(motor_adc_reading);
    //ADC_setArrayCurrent(array_adc_reading);
    ADC_setReading(offset_adc_val, OFFSET_REF_AM__ADC1_IN0);
    ADC_setReading(motor_adc_reading, M_SENSE__ADC3_IN2);
    ADC_setReading(array_adc_reading, A_SENSE__ADC3_IN3);

    int net_current_out =  (int)ADC_netCurrentOut();

    TEST_ASSERT_FLOAT_WITHIN(1.0f, (motor_expected_I-array_expected_I), (float)net_current_out);

}


void test_ADC3_processRawReadings()
{
    uint32_t* battery_first_half = generateSineCurve(100, 10, 400.0, 1000.0);
    uint32_t* battery_second_half = generateSineCurve(100, 10, 400.0, 2000.0);

    uint32_t* motor_first_half = generateSineCurve(100, 10, 750.0, 8000.0);
    uint32_t* motor_second_half = generateSineCurve(100, 10, 750.0, 10000.0);

    uint32_t* array_first_half = generateSineCurve(100, 10, 3000.0, 25000.0);
    uint32_t* array_second_half = generateSineCurve(100, 10, 3000.0, 15000.0);
    
    //uint32_t* adc3_prep_buf_mock = (uint32_t*)malloc(600*sizeof(uint32_t));
    volatile uint32_t* adc3_buf_mock = (uint32_t*)malloc(600*sizeof(uint32_t));

    for (int n = 0; n < 200; n++) {
        adc3_buf_mock[n * 3    ] = (n < 100) ? battery_first_half[n] : battery_second_half[n-100];
        adc3_buf_mock[n * 3 + 1] = (n < 100) ? motor_first_half[n] : motor_second_half[n-100];
        adc3_buf_mock[n * 3 + 2] = (n < 100) ? array_first_half[n] : array_second_half[n-100];

        if (n == 100) printf("---------------------------------------\n");
        printf("%.3d :  %.6d || %.6d || %.6d\n", n, adc3_buf_mock[n*3], adc3_buf_mock[n*3+1], adc3_buf_mock[n*3+2]);
    }

    float expected_vals[] = {1000.0, 8000.0, 25000.0, 2000.0, 10000.0, 15000.0};
    float* actual_vals = calloc(6, sizeof(float));

    ADC3_processRawReadings(0, adc3_buf_mock, actual_vals);
    ADC3_processRawReadings(1, adc3_buf_mock, actual_vals + 3);

    for (int x = 0; x < 6; x++) TEST_ASSERT_FLOAT_WITHIN(1.0f, expected_vals[x], actual_vals[x]);

}

/**
 * @brief This function returns an array of uint32_t values representing a Sine signal.
 * 
 * @param num_points Each point has an arbitrary time step of 1s
 * @param num_cycles Number of complete cycles the signal has
 * 
 * @return uint32_t* 
 */


uint32_t* generateSineCurve(int num_points, int num_cycles, float amplitude, float y_offset)
{
    uint32_t* signal_out = (int*)malloc(sizeof(int)*num_points);
    float total_time = 1.0 * num_points;
    float frequency = ((float)num_cycles/total_time);

    for (int x = 0; x < num_points; x++) {
        signal_out[x] = (uint32_t)((double)amplitude * sin(2.0 * M_PI * (double)frequency * (double)x) + y_offset);
        //printf("%d\n", signal_out[x]);
    }

    return signal_out;
}


float ** enumerateGetSetValsOverRange(int adc_channel, int (*get_function_int)(), float (*get_function_float)(), float start_point[], float end_point[], 
                    int num_samples, float adc_offset, double adc_scale_factor)
{

    float test_plot_gradient = (end_point[1] - start_point[1])/(end_point[0] - start_point[0]);
    float test_plot_intercept = end_point[1] - test_plot_gradient * end_point[0];
    float test_plot_delta_x = (end_point[0] - start_point[0])/(num_samples-1);

    float** results_array = (float**)malloc(2*sizeof(float*));
    results_array[0] = (float*)malloc(num_samples*sizeof(float));
    results_array[1] = (float*)malloc(num_samples*sizeof(float));

    float test_val, expected_val, actual_val;
    int adc_resolution = 4096-1;
    int n = 0;
    
    for (float x = start_point[0]; x <= end_point[0]; x += test_plot_delta_x){
        test_val = test_plot_gradient*x + test_plot_intercept;
        if (test_val < 0) expected_val = adc_offset * adc_scale_factor;
        else if (test_val >= 0 && test_val <= adc_resolution) expected_val = (3.3*(test_val/adc_resolution)+adc_offset)*adc_scale_factor;
        else if (test_val > adc_resolution) expected_val = (3.3+adc_offset)*adc_scale_factor;

        ADC_setReading(test_val, adc_channel);
        actual_val = get_function_int != NULL ? get_function_int() : get_function_float();

        printf("%9.2f : %9.2f || %9.2f\n", test_val, expected_val, actual_val);

        results_array[0][n] = expected_val;
        results_array[1][n] = actual_val;

        n++;
    }

    return results_array;

}

#endif //TEST
