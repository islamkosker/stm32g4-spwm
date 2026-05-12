#include "unity.h"

void test_pwm_phase(void);
void test_pwm_commit(void);
void test_protocol(void);

void setUp(void) {}
void tearDown(void) {}

int main(void)
{
    UNITY_BEGIN();

    test_pwm_phase();
    test_pwm_commit();
    test_protocol();

    return UNITY_END();
}
