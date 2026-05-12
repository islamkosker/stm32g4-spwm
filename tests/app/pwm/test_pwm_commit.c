/**
 * @file test_pwm_commit.c
 * @brief Half/full split commit must touch only the half DMA is NOT reading,
 *        so a single DMA event leaves the other half untouched. After both
 *        events arrive the entire buffer reflects the latest amplitude.
 */

#include "app_pwm.h"
#include "test_common.h"
#include "unity.h"

#define TEST_TIMER TIMER_1

static void seed_buffer_with_pattern(uint32_t pattern)
{
    pwm_group_t* p = &PWM_GROUPS[TEST_TIMER];
    for (size_t i = 0; i < DEFAULT_LUT_SIZE; ++i)
        for (int ch = 0; ch < DEFAULT_PWM_CHANNEL_COUNT; ++ch)
            p->dma_buffer[i][ch] = pattern;
}

static int count_buffer_matching(uint32_t value, pwm_channel_id_t ch, size_t lo, size_t hi)
{
    pwm_group_t* p = &PWM_GROUPS[TEST_TIMER];
    int count = 0;
    for (size_t i = lo; i < hi; ++i)
        if (p->dma_buffer[i][ch] == value) count++;
    return count;
}

void test_commit_half_event_writes_first_half_only(void)
{
    test_common_reset_all();
    pwm_start(TEST_TIMER);

    const uint32_t sentinel = 0xDEADBEEFu;
    seed_buffer_with_pattern(sentinel);

    pwm_set_amplitude(TEST_TIMER, PWM_CH1, PWM_SCALE_MAX, false);
    pwm_irq_dispatch(PWM_TIMER_HANDLES[TEST_TIMER], PWM_EVENT_DMA_HALF);

    /* HALF event = DMA reading second half = first half is the safe write target.
     * Second half of CH1 must still hold the sentinel. */
    int second_half_untouched =
        count_buffer_matching(sentinel, PWM_CH1, DEFAULT_LUT_SIZE / 2, DEFAULT_LUT_SIZE);
    TEST_ASSERT_EQUAL_INT(DEFAULT_LUT_SIZE / 2, second_half_untouched);

    /* First half must NO LONGER be all-sentinel: it's been overwritten. */
    int first_half_untouched = count_buffer_matching(sentinel, PWM_CH1, 0, DEFAULT_LUT_SIZE / 2);
    TEST_ASSERT_LESS_THAN_INT(DEFAULT_LUT_SIZE / 2, first_half_untouched);
}

void test_commit_full_event_writes_second_half_only(void)
{
    test_common_reset_all();
    pwm_start(TEST_TIMER);

    const uint32_t sentinel = 0xCAFEBABEu;
    seed_buffer_with_pattern(sentinel);

    pwm_set_amplitude(TEST_TIMER, PWM_CH1, PWM_SCALE_MAX, false);
    pwm_irq_dispatch(PWM_TIMER_HANDLES[TEST_TIMER], PWM_EVENT_DMA_FULL);

    /* FULL event = DMA wrapped to first half = second half is the safe write target. */
    int first_half_untouched = count_buffer_matching(sentinel, PWM_CH1, 0, DEFAULT_LUT_SIZE / 2);
    TEST_ASSERT_EQUAL_INT(DEFAULT_LUT_SIZE / 2, first_half_untouched);

    int second_half_untouched =
        count_buffer_matching(sentinel, PWM_CH1, DEFAULT_LUT_SIZE / 2, DEFAULT_LUT_SIZE);
    TEST_ASSERT_LESS_THAN_INT(DEFAULT_LUT_SIZE / 2, second_half_untouched);
}

void test_commit_after_both_events_buffer_fully_updated(void)
{
    test_common_reset_all();
    pwm_start(TEST_TIMER);

    const uint32_t scale = 512;
    pwm_set_amplitude(TEST_TIMER, PWM_CH1, false, scale);
    pwm_irq_dispatch(PWM_TIMER_HANDLES[TEST_TIMER], PWM_EVENT_DMA_HALF);
    pwm_irq_dispatch(PWM_TIMER_HANDLES[TEST_TIMER], PWM_EVENT_DMA_FULL);

    pwm_group_t* p = &PWM_GROUPS[TEST_TIMER];
    for (size_t i = 0; i < DEFAULT_LUT_SIZE; ++i)
    {
        uint32_t expected = (p->lut[i] * scale) >> PWM_SCALE_SHIFT;
        TEST_ASSERT_EQUAL_UINT32(expected, p->dma_buffer[i][PWM_CH1]);
    }

    /* Pending flag must clear once both halves have been seen. */
    TEST_ASSERT_FALSE(p->pwm_channels[PWM_CH1].update_pending);
    TEST_ASSERT_EQUAL_UINT8(0, p->pwm_channels[PWM_CH1].commit_mask);
}

void test_pwm_commit(void)
{
    RUN_TEST(test_commit_half_event_writes_first_half_only);
    RUN_TEST(test_commit_full_event_writes_second_half_only);
    RUN_TEST(test_commit_after_both_events_buffer_fully_updated);
}
