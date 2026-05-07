/**
 * @file pwm_bindings.c
 * @brief
 * @author islamkosker (https://github.com/islamkosker)
 * @date 2026-05-07 22:08
 * @version 1.0.0
 * @copyright Copyright (c) 2026 Islam Kosker.
 */

#include "app_pwm_config.h"
#include "interface.h"
#include "stm32g4xx_hal.h"


extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim8;

hal_abc_ptr_t PWM_TIMER_HANDLES[TIMER_COUNT] = {
#define X(id, hw, handle) [id] = &(handle),
    TIMER_LIST};
#undef X