/**
 * @file interface_pwm.h
 * @brief
 * @author islamkosker (https://github.com/islamkosker)
 * @date 2026-05-06 21:50
 * @version 1.0.0
 * @copyright Copyright (c) 2026 Islam Kosker.
 */

#ifndef INTERFACE_PWM_H
#define INTERFACE_PWM_H
#include "interface.h"
#include <stddef.h>
#include <stdint.h>

typedef enum
{
    INTF_PWM_CH1 = 0,
    INTF_PWM_CH2,
    INTF_PWM_CH3,
    INTF_PWM_CH4,
} interface_pwm_channel_t;

interface_status_t interface_pwm_start_dma_burst(hal_abc_ptr_t handle, uint32_t* buffer,
                                                 uint32_t burst_len, const size_t buffer_len);
interface_status_t interface_pwm_stop_dma_burst(hal_abc_ptr_t handle);
interface_status_t interface_pwm_start(hal_abc_ptr_t handle, interface_pwm_channel_t channel_id);
interface_status_t interface_pwm_stop(hal_abc_ptr_t handle, interface_pwm_channel_t channel_id);
interface_status_t interface_pwm_set_compare(hal_abc_ptr_t handle, interface_pwm_channel_t channel_id,
                                             uint32_t value);
#endif