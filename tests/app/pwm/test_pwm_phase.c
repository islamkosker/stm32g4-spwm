/**
 * @file test_pwm_phase.c
 * @brief Verifies that pwm_set_amplitude(..., phase_inverted=true) makes
 *        the DMA buffer read the LUT shifted by half a cycle (180 deg).
 *        Tests rely on the public PWM API + IRQ dispatch path: a START
 *        followed by two simulated DMA events flushes both halves.
 */

#include "app_pwm.h"
#include "test_common.h"
#include "unity.h"

#define TEST_TIMER TIMER_1

static void apply_amplitude_and_flush(pwm_channel_id_t ch, uint32_t scale, bool phase)
{
    pwm_set_amplitude(TEST_TIMER, ch, phase, scale);
    /* pwm_channel_on_dma_event needs both events to land before
     * update_pending is cleared, regardless of order. */
    pwm_irq_dispatch(PWM_TIMER_HANDLES[TEST_TIMER], PWM_EVENT_DMA_HALF);
    pwm_irq_dispatch(PWM_TIMER_HANDLES[TEST_TIMER], PWM_EVENT_DMA_FULL);
}

void test_phase_inverted_shifts_lut_by_half(void)
{
    test_common_reset_all();
    pwm_start(TEST_TIMER);
    pwm_set_amplitude(TEST_TIMER, 0, 0, 1024);
    const uint32_t scale = PWM_SCALE_MAX;
    apply_amplitude_and_flush(PWM_CH1, scale, true);

    pwm_group_t* p = &PWM_GROUPS[TEST_TIMER];
    for (size_t i = 0; i < DEFAULT_LUT_SIZE; ++i)
    {
        uint32_t expected = test_common_expected_dma(p->lut, i, scale, true);
        TEST_ASSERT_EQUAL_UINT32(expected, p->dma_buffer[i][PWM_CH1]);
    }

    uint32_t expected0 = (p->lut[DEFAULT_LUT_SIZE / 2] * scale) >> PWM_SCALE_SHIFT;
    TEST_ASSERT_EQUAL_UINT32(expected0, p->dma_buffer[0][PWM_CH1]);
}

void test_phase_inphase_matches_lut_directly(void)
{
    test_common_reset_all();
    pwm_start(TEST_TIMER);

    const uint32_t scale = PWM_SCALE_MAX / 2;
    apply_amplitude_and_flush(PWM_CH2, scale, false);

    pwm_group_t* p = &PWM_GROUPS[TEST_TIMER];
    for (size_t i = 0; i < DEFAULT_LUT_SIZE; ++i)
    {
        uint32_t expected = (p->lut[i] * scale) >> PWM_SCALE_SHIFT;
        TEST_ASSERT_EQUAL_UINT32(expected, p->dma_buffer[i][PWM_CH2]);
    }
}

void test_phase_inversion_can_be_cleared(void)
{
    test_common_reset_all();
    pwm_start(TEST_TIMER);

    apply_amplitude_and_flush(PWM_CH1, PWM_SCALE_MAX, true);
    apply_amplitude_and_flush(PWM_CH1, PWM_SCALE_MAX, false);

    pwm_group_t* p = &PWM_GROUPS[TEST_TIMER];
    /* After clearing phase, dma[0] is back to the in-phase value. */
    uint32_t expected0 = (p->lut[0] * PWM_SCALE_MAX) >> PWM_SCALE_SHIFT;
    TEST_ASSERT_EQUAL_UINT32(expected0, p->dma_buffer[0][PWM_CH1]);
    TEST_ASSERT_FALSE(p->pwm_channels[PWM_CH1].phase_inverted);
}

void test_pwm_phase(void)
{
    RUN_TEST(test_phase_inverted_shifts_lut_by_half);
    RUN_TEST(test_phase_inphase_matches_lut_directly);
    RUN_TEST(test_phase_inversion_can_be_cleared);
}
