/**
 * @file app_pwm.h
 * @brief
 * @author islamkosker (https://github.com/islamkosker)
 * @date 2026-05-07 20:27
 * @version 1.0.0
 * @copyright Copyright (c) 2026 Islam Kosker.
 */

#ifndef APP_PWM_H
#define APP_PWM_H

#include "app_pwm_config.h"
#include "interface_pwm.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define PWM_SAMPLE_COUNT 100
#define DEFAULT_PWM_CHANNEL_COUNT 2
#define DEFAULT_LUT_SIZE 100
#define PWM_SCALE_SHIFT 10
#define PWM_SCALE_MAX 1024U
#define DEFAULT_PSC 1
#define DEFAULT_ARR 392
#define T10KHZ_ARR 169

extern const uint32_t LUT_2_5KHZ[DEFAULT_LUT_SIZE];
extern const uint32_t LUT_3KHZ[DEFAULT_LUT_SIZE];
extern const uint32_t LUT_10KHZ[DEFAULT_LUT_SIZE];

typedef enum
{
    PWM_CH1 = 0,
    PWM_CH2
} pwm_channel_id_t;

typedef enum
{
    PWM_EVENT_DMA_HALF,
    PWM_EVENT_DMA_FULL
} pwm_event_t;

typedef struct
{
    uint32_t psc; // timer prescaler
    uint32_t arr; // timer auto-reload (period)
} timer_config_t;

typedef struct
{
    pwm_channel_id_t ch_id;
    uint32_t scale;
    uint32_t pending_scale;
    bool phase_inverted;
    bool pending_phase_inverted;
    bool active;         /* true while DMA + PWM is running on this channel */
    bool update_pending; /* host requested a new amplitude/phase, not yet fully committed */
    uint8_t commit_mask; /* bit 0 = first half written, bit 1 = second half written */
} pwm_channel_t;

typedef struct
{
    uint32_t (*dma_buffer)[DEFAULT_PWM_CHANNEL_COUNT];
    const uint32_t* lut;
    const timer_config_t* timer_config;
    freq_config_id_t freq_id; /* selected PWM_CONFIGS / LUT profile, for telemetry */
    hal_abc_ptr_t timer;
    pwm_channel_t pwm_channels[DEFAULT_PWM_CHANNEL_COUNT];
    bool freq_update_pending;
} pwm_group_t;

extern pwm_group_t PWM_GROUPS[TIMER_COUNT];
extern hal_abc_ptr_t PWM_TIMER_HANDLES[TIMER_COUNT];
extern const timer_config_t PWM_CONFIGS[SINE_FREQ_COUNT];


void pwm_init(void);
void pwm_set_frequency(timer_id_t t, freq_config_id_t freq);
void pwm_set_frequency_live(timer_id_t t, freq_config_id_t freq);
void pwm_set_amplitude(timer_id_t t, pwm_channel_id_t ch, uint32_t scale, bool phase_inverted);
freq_config_id_t pwm_get_frequency_id(timer_id_t t);
void pwm_start(timer_id_t t);
void pwm_stop(timer_id_t t);
void pwm_irq_dispatch(hal_abc_ptr_t tim, pwm_event_t event);

#endif