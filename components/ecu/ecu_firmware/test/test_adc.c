#include <unity.h>
#include <stdlib.h>
#include <stdio.h>

#include "adc.h"

//#include "mock_stm32f1xx_hal.h"
float ** enumerateGetSetValsOverRange(float (*setFunction)(float), int (*getFunction)(), float start_point[], float end_point[], int num_samples, 
                                        float adc_offset, float adc_scale_factor);

void setUp(void)
{

}

void tearDown(void)
{

}

void test_ADC3_getSetSuppBatteryVoltage(){

    int num_samples = 20;
    float start_point[] = {-4.0, -1400.0};
    float end_point[] = {15.0, 5250.0};

    float** test_results = enumerateGetSetValsOverRange(ADC3_setSuppBattVoltage, ADC3_getSuppBattVoltage, start_point, end_point, 20,
                            0.0, 11.0*1000);
    for (int n = 0; n < num_samples; n++) TEST_ASSERT_FLOAT_WITHIN(1.0f, test_results[0][n], test_results[1][n]);

}

void test_ADC3_getSetMotorCurrent(){
    
    int num_samples = 20;
    float start_point[] = {-4.0, -1400.0};
    float end_point[] = {15.0, 5250.0};

    float** test_results = enumerateGetSetValsOverRange(ADC3_setMotorCurrent, ADC3_getMotorCurrent, start_point, end_point, 20,
                            -2.5, 100.0*80.0);
    for (int n = 0; n < num_samples; n++) TEST_ASSERT_FLOAT_WITHIN(1.0f, test_results[0][n], test_results[1][n]);

}

void test_ADC3_getSetArrayCurrent(){

    int num_samples = 20;
    float start_point[] = {-4.0, -1400.0};
    float end_point[] = {15.0, 5250.0};

    float** test_results = enumerateGetSetValsOverRange(ADC3_setArrayCurrent, ADC3_getArrayCurrent, start_point, end_point, 20,
                            -2.5, 100.0*80.0);
    for (int n = 0; n < num_samples; n++) TEST_ASSERT_FLOAT_WITHIN(1.0f, test_results[0][n], test_results[1][n]);

}

void test_ADC3_netCurrentOut(){
    
    int adc_resolution = 4096-1;

    int motor_adc_reading = 3250;
    int array_adc_reading = 3150;

    float motor_expected_I = (((float)motor_adc_reading*3.3/adc_resolution)-2.5)*100*80;
    float array_expected_I = (((float)array_adc_reading*3.3/adc_resolution)-2.5)*100*80;

    ADC3_setMotorCurrent(motor_adc_reading);
    ADC3_setArrayCurrent(array_adc_reading);

    int net_current_out =  (int)ADC3_netCurrentOut();

    TEST_ASSERT_FLOAT_WITHIN(1.0f, (motor_expected_I-array_expected_I), net_current_out);

}



float ** enumerateGetSetValsOverRange(float (*setFunction)(float), int (*getFunction)(), float start_point[], float end_point[], int num_samples, 
                    float adc_offset, float adc_scale_factor){

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

        setFunction(test_val);
        actual_val = (int)getFunction();

        results_array[0][n] = expected_val;
        results_array[1][n] = actual_val;

        n++;
    }

    return results_array;

}

