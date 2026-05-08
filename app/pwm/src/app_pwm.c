/**
 * @file app_pwm.c
 * @brief
 * @author islamkosker (https://github.com/islamkosker)
 * @date 2026-05-07 21:13
 * @version 1.0.0
 * @copyright Copyright (c) 2026 Islam Kosker.
 */

#include "app_pwm.h"
#include "interface_pwm.h"
#include "interface_timer.h"

#define COMMIT_PENDING 0U
#define PWM_COMMIT_HALF (1u << 0)
#define PWM_COMMIT_FULL (1u << 1)
#define PWM_COMMIT_DONE (PWM_COMMIT_HALF | PWM_COMMIT_FULL)
#define VREF_CALIB(s) (((s) >> 1) + 24 + ((int32_t)((s) - 512) * 14) / 512)

static const uint32_t* get_lut(const freq_config_id_t freq)
{
    switch (freq)
    {
    case SINE_FREQ_2K5HZ: return LUT_2_5KHZ;
    case SINE_FREQ_3KHZ: return LUT_3KHZ;
    case SINE_FREQ_10KHZ: return LUT_10KHZ;

    default: return NULL;
    }
}

static inline void set_frequency(pwm_group_t* const pwm_grp, const freq_config_id_t freq)
{
    pwm_grp->lut = get_lut(freq);
    pwm_grp->timer_config = &PWM_CONFIGS[freq];
    pwm_grp->freq_id = freq;
}

static void update_vref_out(pwm_group_t* const pwm_grp, const pwm_channel_id_t ch)
{
    uint32_t scale = pwm_grp->pwm_channels[ch].scale;

    uint32_t value = (pwm_grp->timer_config->arr * (VREF_CALIB(scale))) >> PWM_SCALE_SHIFT;
    if (ch == PWM_CH1) interface_pwm_set_compare(pwm_grp->timer, INTF_PWM_CH3, value);

    if (ch == PWM_CH2) interface_pwm_set_compare(pwm_grp->timer, INTF_PWM_CH4, value);
}

static void update_dma_buffer_range(pwm_group_t* const pwm_grp, const pwm_channel_id_t chidx,
                                    const size_t start, const size_t end)
{
    pwm_channel_t* c = &pwm_grp->pwm_channels[chidx];
    uint32_t scale = c->scale;
    size_t off = c->phase_inverted ? (DEFAULT_LUT_SIZE / 2) : 0;

    for (size_t i = start; i < end; ++i)
    {
        size_t idx = (i + off) % DEFAULT_LUT_SIZE;
        pwm_grp->dma_buffer[i][chidx] = (pwm_grp->lut[idx] * scale) >> PWM_SCALE_SHIFT;
    }
}

static void fill_dma_buffer(pwm_group_t* const pwm_grp)
{
    for (int ch = 0; ch < DEFAULT_PWM_CHANNEL_COUNT; ch++)
    {
        update_dma_buffer_range(pwm_grp, (pwm_channel_id_t)ch, 0, DEFAULT_LUT_SIZE);
    }
}

static void setup_timer_config(pwm_group_t* const pwm_grp)
{
    interface_timer_set_arr(pwm_grp->timer, pwm_grp->timer_config->arr);
    interface_timer_set_psc(pwm_grp->timer, pwm_grp->timer_config->psc);
}

static interface_status_t start_dma(pwm_group_t* const pwm_grp)
{
    INTERFACE_RETURN_IF_ERROR(interface_pwm_start_dma_burst(
        pwm_grp->timer, &pwm_grp->dma_buffer[0][0], DEFAULT_PWM_CHANNEL_COUNT,
        DEFAULT_LUT_SIZE * DEFAULT_PWM_CHANNEL_COUNT));

    INTERFACE_RETURN_IF_ERROR(interface_pwm_start(pwm_grp->timer, INTF_PWM_CH1));
    INTERFACE_RETURN_IF_ERROR(interface_pwm_start(pwm_grp->timer, INTF_PWM_CH2));
    INTERFACE_RETURN_IF_ERROR(interface_pwm_start(pwm_grp->timer, INTF_PWM_CH3));
    INTERFACE_RETURN_IF_ERROR(interface_pwm_start(pwm_grp->timer, INTF_PWM_CH4));
    pwm_grp->pwm_channels[0].active = true;
    pwm_grp->pwm_channels[1].active = true;

    return INTERFACE_STATUS_OK;
}

static interface_status_t stop_dma(pwm_group_t* const pwm_grp)
{
    pwm_grp->pwm_channels[0].active = false;
    pwm_grp->pwm_channels[1].active = false;

    INTERFACE_RETURN_IF_ERROR(interface_pwm_stop_dma_burst(pwm_grp->timer));

    INTERFACE_RETURN_IF_ERROR(interface_pwm_stop(pwm_grp->timer, INTF_PWM_CH1));
    INTERFACE_RETURN_IF_ERROR(interface_pwm_stop(pwm_grp->timer, INTF_PWM_CH2));
    INTERFACE_RETURN_IF_ERROR(interface_pwm_stop(pwm_grp->timer, INTF_PWM_CH3));
    INTERFACE_RETURN_IF_ERROR(interface_pwm_stop(pwm_grp->timer, INTF_PWM_CH4));

    return INTERFACE_STATUS_OK;
}

void pwm_init(void)
{
    for (int t = 0; t < TIMER_COUNT; t++)
    {
        PWM_GROUPS[t].dma_buffer = DMA_BUFFER_PTR[t];
        PWM_GROUPS[t].timer = PWM_TIMER_HANDLES[t];
        PWM_GROUPS[t].lut = LUT_3KHZ;
        PWM_GROUPS[t].timer_config = &PWM_CONFIGS[SINE_FREQ_3KHZ];
        PWM_GROUPS[t].freq_id = SINE_FREQ_3KHZ;

        PWM_GROUPS[t].pwm_channels[0] = (pwm_channel_t){
            .ch_id = PWM_CH1,
            .scale = PWM_SCALE_MAX / 2,
            .phase_inverted = false,
        };
        PWM_GROUPS[t].pwm_channels[1] = (pwm_channel_t){
            .ch_id = PWM_CH2,
            .scale = PWM_SCALE_MAX / 2,
            .phase_inverted = false,
        };
    }
}

void pwm_start(const timer_id_t t)
{
    pwm_group_t* p = &PWM_GROUPS[t];
    fill_dma_buffer(p);
    setup_timer_config(p);

    for (int ch = 0; ch < DEFAULT_PWM_CHANNEL_COUNT; ch++)
    {
        update_vref_out(p, (pwm_channel_id_t)ch);
    }

    start_dma(p);
}

void pwm_stop(const timer_id_t t)
{
    pwm_group_t* p = &PWM_GROUPS[t];
    stop_dma(p);
}

void pwm_set_frequency(const timer_id_t t, const freq_config_id_t freq)
{
    pwm_group_t* p = &PWM_GROUPS[t];

    const bool was_runnig = p->pwm_channels[0].active || p->pwm_channels[1].active;

    if (was_runnig) pwm_stop(t);

    set_frequency(p, freq);
    if (was_runnig) pwm_start(t);
}

freq_config_id_t pwm_get_frequency_id(const timer_id_t t)
{
    if (t >= TIMER_COUNT) return SINE_FREQ_COUNT; // in error
    return PWM_GROUPS[t].freq_id;
}


void pwm_set_amplitude(const timer_id_t t, const pwm_channel_id_t ch, const bool phase_inverted,
                       uint32_t scale)
{
    if (t >= TIMER_COUNT || ch >= DEFAULT_PWM_CHANNEL_COUNT) return;

    pwm_group_t* const p = &PWM_GROUPS[t];
    pwm_channel_t* const c = &p->pwm_channels[ch];

    if (scale > PWM_SCALE_MAX) scale = PWM_SCALE_MAX;

    c->pending_scale = scale;
    c->pending_phase_inverted = phase_inverted;
    c->commit_mask = COMMIT_PENDING;
    c->update_pending = true;
}

static void pwm_channel_on_dma_event(pwm_group_t* const p, const pwm_channel_id_t ch,
                                     const pwm_event_t evt)
{
    pwm_channel_t* c = &p->pwm_channels[ch];

    if (!c->active) return;

    if (!c->update_pending) return;

    if (c->commit_mask == COMMIT_PENDING) 
    {
        c->scale = c->pending_scale;
        c->phase_inverted = c->pending_phase_inverted;
    }

    if (evt == PWM_EVENT_DMA_HALF)
    {
        update_dma_buffer_range(p, ch, 0, DEFAULT_LUT_SIZE / 2);
        c->commit_mask |= PWM_COMMIT_HALF;
    }
    else if (evt == PWM_EVENT_DMA_FULL)
    {
        update_dma_buffer_range(p, ch, DEFAULT_LUT_SIZE / 2, DEFAULT_LUT_SIZE);
        c->commit_mask |= PWM_COMMIT_FULL;
    }

    update_vref_out(p, ch);

    /* Pending only cleared once both halves have been committed. */
    if (c->commit_mask == PWM_COMMIT_DONE)
    {
        c->update_pending = false;
        c->commit_mask = COMMIT_PENDING;
    }
}

void pwm_irq_dispatch(hal_abc_ptr_t const tim, const pwm_event_t event)
{
    for (size_t i = 0; i < TIMER_COUNT; i++)
    {
        pwm_group_t* p = &PWM_GROUPS[i];

        if (p->timer != tim) continue;

        for (int ch = 0; ch < DEFAULT_PWM_CHANNEL_COUNT; ch++)
        {
            pwm_channel_on_dma_event(p, (pwm_channel_id_t)ch, event);
        }

        break;
    }
}
