/**
 * @file app_pwm_config.h
 * @brief
 * @author islamkosker (https://github.com/islamkosker)
 * @date 2026-05-07 21:31
 * @version 1.0.0
 * @copyright Copyright (c) 2026 Islam Kosker.
 */

#ifndef APP_PWM_CONFIG_H
#define APP_PWM_CONFIG_H

#include <stdint.h>

#define DEFAULT_PWM_CHANNEL_COUNT 2
#define DEFAULT_LUT_SIZE 100

#define ARR_2K5HZ 1
#define PSC_2K5HZ 399

#define ARR_3KHZ 1
#define PSC_3KHZ 282

#define ARR_10KHZ 0
#define PSC_10KHZ 170

#define TIMER_LIST X(TIMER_1, TIM1, htim1)
// X(TIMER_2, TIM2, htim2)
// X(TIMER_3, TIM3, htim3)
// X(TIMER_4, TIM4, htim4)
// X(TIMER_8, TIM8, htim8)

#define TIMER_CONFIG_LIST                                                                               \
    X(SINE_FREQ_2K5HZ, ARR_2K5HZ, PSC_2K5HZ)                                                            \
    X(SINE_FREQ_3KHZ, ARR_3KHZ, PSC_3KHZ)                                                               \
    X(SINE_FREQ_10KHZ, ARR_10KHZ, PSC_10KHZ)

typedef enum
{
#define X(id, hw, handle) id,
    TIMER_LIST
#undef X
        TIMER_COUNT

} timer_id_t;

#define X(id, hw, handle) extern uint32_t DMA_BUFFER_##hw[DEFAULT_LUT_SIZE][DEFAULT_PWM_CHANNEL_COUNT];

TIMER_LIST

#undef X

extern uint32_t (*DMA_BUFFER_PTR[TIMER_COUNT])[DEFAULT_PWM_CHANNEL_COUNT];

typedef enum
{
#define X(id, arr, psc) id,
    TIMER_CONFIG_LIST
#undef X
        SINE_FREQ_COUNT
} freq_config_id_t;

#endif