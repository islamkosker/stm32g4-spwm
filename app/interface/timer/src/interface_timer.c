/**
 * @file interface_timer.c
 * @brief
 * @author islamkosker (https://github.com/islamkosker)
 * @date 2026-05-08 19:10
 * @version 1.0.0
 * @copyright Copyright (c) 2026 Islam Kosker.
 */

#include "interface_timer.h"
#include "stm32g4xx_hal.h"

void interface_timer_set_arr(hal_abc_ptr_t handle, uint32_t arr)
{
    TIM_HandleTypeDef* timer = (TIM_HandleTypeDef*)handle;

    __HAL_TIM_SET_AUTORELOAD(timer, arr);
}
void interface_timer_set_psc(hal_abc_ptr_t handle, uint32_t psc)
{
    TIM_HandleTypeDef* timer = (TIM_HandleTypeDef*)handle;

    __HAL_TIM_SET_PRESCALER(timer, psc);
}
