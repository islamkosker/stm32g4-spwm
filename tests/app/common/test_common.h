#ifndef TEST_COMMON_H_
#define TEST_COMMON_H_

#include "app_pwm.h"
#include "pwm_mocks.h"
#include "timer_mocks.h"
#include <stdint.h>

/* Bring all PWM groups + mocks back to a known state before each test. */
static inline void test_common_reset_all(void)
{
    mock_pwm_reset();
    mock_timer_reset();
    pwm_init();
}

/* Compute the value pwm_channel_on_dma_event / fill_dma_buffer would write
 * into dma_buffer[i][ch] for the given scale and phase flag. Mirrors the
 * implementation under test, so a mismatch flags a regression. */
static inline uint32_t test_common_expected_dma(const uint32_t* lut, size_t i, uint32_t scale,
                                                bool phase_inverted)
{
    size_t off = phase_inverted ? (DEFAULT_LUT_SIZE / 2) : 0;
    size_t idx = (i + off) % DEFAULT_LUT_SIZE;
    return (lut[idx] * scale) >> PWM_SCALE_SHIFT;
}

#endif /* TEST_COMMON_H_ */
