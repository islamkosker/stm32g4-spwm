/**
 * @file interface_stubs.c
 * @brief Test-only stubs that satisfy interface_pwm_impl.h /
 *        interface_timer_impl.h externs and the pwm_bindings.c equivalents
 *        (PWM_TIMER_HANDLES) when running unit tests on the host. The real
 *        implementations live in app/interface/pwm/src/interface_pwm.c and
 *        app/pwm/src/pwm_bindings.c, which depend on the STM32 HAL.
 */

#include <string.h>

#include "app_pwm.h"
#include "app_pwm_config.h"
#include "pwm_mocks.h"
#include "timer_mocks.h"

/* --- mock state --------------------------------------------------------- */

/* --- pwm interface mock impls ------------------------------------------- */
pwm_mock_state_t g_pwm_mock_state;
timer_mock_state_t g_timer_mock_state;

static int s_dummy_handles[TIMER_COUNT];
hal_abc_ptr_t PWM_TIMER_HANDLES[TIMER_COUNT] = {[TIMER_1] = &s_dummy_handles[0]};

interface_status_t mock_pwm_dma_burst_start(hal_abc_ptr_t handle, uint32_t* buffer, uint32_t burst_len,
                                            uint32_t buffer_len)
{
    g_pwm_mock_state.dma_burst_start_called++;
    g_pwm_mock_state.last_dma_handle = handle;
    g_pwm_mock_state.last_dma_buffer = buffer;
    g_pwm_mock_state.last_dma_burst_len = burst_len;
    g_pwm_mock_state.last_dma_buffer_len = buffer_len;
    return INTERFACE_STATUS_OK;
}

interface_status_t mock_pwm_dma_burst_stop(hal_abc_ptr_t handle)
{
    (void)handle;
    g_pwm_mock_state.dma_burst_stop_called++;
    return INTERFACE_STATUS_OK;
}

interface_status_t mock_pwm_stop(hal_abc_ptr_t handle, uint32_t channel_id)
{
    (void)handle;
    (void)channel_id;
    g_pwm_mock_state.stop_called++;
    return INTERFACE_STATUS_OK;
}

interface_status_t mock_pwm_start(hal_abc_ptr_t handle, uint32_t channel_id)
{
    (void)handle;
    (void)channel_id;
    g_pwm_mock_state.start_called++;
    return INTERFACE_STATUS_OK;
}

interface_status_t mock_pwm_set_compare(hal_abc_ptr_t handle, uint32_t channel_id, uint32_t value)
{
    (void)handle;
    g_pwm_mock_state.set_compare_called++;
    if (channel_id <= INTF_PWM_CH4)
    {
        g_pwm_mock_state.last_compare_value[channel_id] = value;
        g_pwm_mock_state.compare_call_count[channel_id]++;
    }
    return INTERFACE_STATUS_OK;
}

void mock_pwm_reset(void) { memset(&g_pwm_mock_state, 0, sizeof(g_pwm_mock_state)); }

/* --- timer interface mock impls ----------------------------------------- */

void mock_timer_change_psc(hal_abc_ptr_t handle, uint32_t psc)
{
    g_timer_mock_state.change_psc_called++;
    g_timer_mock_state.last_handle = handle;
    g_timer_mock_state.last_psc = psc;
}

void mock_timer_change_arr(hal_abc_ptr_t handle, uint32_t arr)
{
    g_timer_mock_state.change_arr_called++;
    g_timer_mock_state.last_handle = handle;
    g_timer_mock_state.last_arr = arr;
}

void mock_timer_reset(void) { memset(&g_timer_mock_state, 0, sizeof(g_timer_mock_state)); }

interface_status_t interface_pwm_start_dma_burst(hal_abc_ptr_t handle, uint32_t* buffer,
                                                 uint32_t burst_len, const size_t buffer_len)
{
    return mock_pwm_dma_burst_start(handle, buffer, burst_len, buffer_len);
}

interface_status_t interface_pwm_stop_dma_burst(hal_abc_ptr_t handle)
{
    return mock_pwm_dma_burst_stop(handle);
}

interface_status_t interface_pwm_start(hal_abc_ptr_t handle, interface_pwm_channel_t channel_id)
{
    return mock_pwm_start(handle, channel_id);
}

interface_status_t interface_pwm_stop(hal_abc_ptr_t handle, interface_pwm_channel_t channel_id)
{
    return mock_pwm_stop(handle, channel_id);
}

interface_status_t interface_pwm_set_compare(hal_abc_ptr_t handle, interface_pwm_channel_t channel_id,
                                             uint32_t value)
{
    return mock_pwm_set_compare(handle, channel_id, value);
}

void interface_timer_set_arr(hal_abc_ptr_t handle, uint32_t arr) { mock_timer_change_arr(handle, arr); }
void interface_timer_set_psc(hal_abc_ptr_t handle, uint32_t psc) { mock_timer_change_psc(handle, psc); }