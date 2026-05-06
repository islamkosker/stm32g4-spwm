/**
 * @file interface_pwm.c
 * @brief
 * @author islamkosker (https://github.com/islamkosker)
 * @date 2026-05-06 22:17
 * @version 1.0.0
 * @copyright Copyright (c) 2026 Islam Kosker.
 */

#include "interface_pwm.h"
#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_tim.h"
#include <limits.h>

/**
 * @brief Get DMA burst length macro based on channel count
 *
 * @param channel_count Number of PWM channels (1–4)
 * @return Corresponding HAL macro TIM_DMABURSTLENGTH_XTRANSFERS
 * @note Returns UINT32_MAX if the channel count is invalid
 */
static inline uint32_t get_burst_length(size_t channel_count)
{
    switch (channel_count)
    {
    case 1: return TIM_DMABURSTLENGTH_1TRANSFER;
    case 2: return TIM_DMABURSTLENGTH_2TRANSFERS;
    case 3: return TIM_DMABURSTLENGTH_3TRANSFERS;
    case 4: return TIM_DMABURSTLENGTH_4TRANSFERS;
    default: return UINT32_MAX; // Invalid channel count
    }
}

/**
 * @brief Convert interface PWM channel to HAL TIM channel macro
 *
 * @param ch PWM channel identifier (interface_pwm_channel_t)
 * @return Corresponding HAL macro TIM_CHANNEL_X
 * @note Returns UINT32_MAX if the channel is invalid
 */
static inline uint32_t to_hal_channel(interface_pwm_channel_t ch)
{
    switch (ch)
    {
    case INTF_PWM_CH1: return TIM_CHANNEL_1;
    case INTF_PWM_CH2: return TIM_CHANNEL_1;
    case INTF_PWM_CH3: return TIM_CHANNEL_1;
    case INTF_PWM_CH4: return TIM_CHANNEL_1;
    default: return UINT32_MAX;
    }
}

interface_status_t interface_pwm_start_dma_burst(hal_abc_ptr_t handle, uint32_t* buffer,
                                                 uint32_t burst_len, const size_t buffer_len)
{
    if (!handle || !buffer || burst_len == 0 || buffer_len == 0) return INTERFACE_STATUS_INVALID_ARG;

    uint32_t burst_length = get_burst_length(burst_len);

    HAL_StatusTypeDef ret = HAL_OK;
    ret = HAL_TIM_DMABurst_MultiWriteStart(handle, TIM_DMABASE_CCR1, TIM_DMA_UPDATE, buffer,
                                           burst_length, buffer_len);
    return (ret == HAL_OK) ? INTERFACE_STATUS_OK : INTERFACE_STATUS_ERROR;
}

interface_status_t interface_pwm_stop_dma_burst(hal_abc_ptr_t handle)
{
    HAL_StatusTypeDef ret = HAL_OK;

    ret = HAL_TIM_DMABurst_WriteStop(handle, TIM_DMA_UPDATE);

    return (ret == HAL_OK) ? INTERFACE_STATUS_OK : INTERFACE_STATUS_ERROR;
}

interface_status_t interface_pwm_start(hal_abc_ptr_t handle, interface_pwm_channel_t channel_id)
{
    if (!handle) return INTERFACE_STATUS_INVALID_ARG;

    HAL_StatusTypeDef ret = HAL_OK;
    ret = HAL_TIM_PWM_Start(handle, to_hal_channel(channel_id));

    return (ret == HAL_OK) ? INTERFACE_STATUS_OK : INTERFACE_STATUS_ERROR;
}

interface_status_t interface_pwm_stop(hal_abc_ptr_t handle, interface_pwm_channel_t channel_id)
{
    if (!handle) return INTERFACE_STATUS_INVALID_ARG;

    HAL_StatusTypeDef ret = HAL_OK;
    ret = HAL_TIM_PWM_Stop(handle, to_hal_channel(channel_id));

    return (ret == HAL_OK) ? INTERFACE_STATUS_OK : INTERFACE_STATUS_ERROR;
}

interface_status_t interface_pwm_set_compare(hal_abc_ptr_t handle, interface_pwm_channel_t channel_id,
                                             uint32_t value)
{
    if (!handle) return INTERFACE_STATUS_INVALID_ARG;

    __HAL_TIM_SET_COMPARE((TIM_HandleTypeDef*)handle, to_hal_channel(channel_id), value);
    return INTERFACE_STATUS_OK;
}