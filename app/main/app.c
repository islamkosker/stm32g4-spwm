#include "app_protocol.h"

#include "app_pwm.h"

void app(void)
{

    pwm_init();

    protocol_dispatcher_init();

    pwm_set_frequency(TIMER_1, SINE_FREQ_3KHZ);

    pwm_start(TIMER_1);

}