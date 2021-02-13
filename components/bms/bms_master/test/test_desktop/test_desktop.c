#ifdef UNIT_TEST

#include <selftest.h>
#include <unity.h>
#include <calculator.h>

void test_add(void) {
    TEST_ASSERT_EQUAL(2, add(1,1));
}

void test_multiply(void) {
    TEST_ASSERT_EQUAL(45, multiply(9,5));
}

void test_itmpConversion(void) {
    uint16_t testTemp[] = {0x595C};
    float tempCelsius[1];
    itmpConversion(testTemp, tempCelsius);
    TEST_ASSERT_EQUAL(25.0, tempCelsius[1]);
}

int main(int argc, char **argv){
    UNITY_BEGIN();
    RUN_TEST(test_add);
    RUN_TEST(test_multiply);
    RUN_TEST(test_itmpConversion);
    UNITY_END();

    return 0;
}
#endif