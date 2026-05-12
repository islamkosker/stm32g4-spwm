#ifndef MOCK_PWM_H
#define MOCK_PWM_H

#include "interface_pwm.h"
#include <stdint.h>
#include <string.h>

#define MOCK_PWM_MAX_CALLS 64

typedef struct
{
    int start_called;
    int stop_called;
    int dma_burst_start_called;
    int dma_burst_stop_called;
    int set_compare_called;

    uint32_t last_compare_value[INTF_PWM_CH4 + 1];
    int compare_call_count[INTF_PWM_CH4 + 1];

    hal_abc_ptr_t last_dma_handle;
    uint32_t* last_dma_buffer;
    uint32_t last_dma_burst_len;
    uint32_t last_dma_buffer_len;
} pwm_mock_state_t;

extern pwm_mock_state_t g_pwm_mock_state;

void mock_pwm_reset(void);
interface_status_t mock_pwm_dma_burst_start(hal_abc_ptr_t handle, uint32_t* buffer, uint32_t burst_len,
                                            uint32_t buffer_len);
interface_status_t mock_pwm_dma_burst_stop(hal_abc_ptr_t handle);
interface_status_t mock_pwm_stop(hal_abc_ptr_t handle, uint32_t channel_id);
interface_status_t mock_pwm_start(hal_abc_ptr_t handle, uint32_t channel_id);
interface_status_t mock_pwm_set_compare(hal_abc_ptr_t handle, uint32_t channel_id, uint32_t value);


#endif /* MOCK_PWM_H */
