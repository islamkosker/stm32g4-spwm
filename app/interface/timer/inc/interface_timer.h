/**
 * @file interface_timer.h
 * @brief
 * @author islamkosker (https://github.com/islamkosker)
 * @date 2026-05-08 19:08
 * @version 1.0.0
 * @copyright Copyright (c) 2026 Islam Kosker.
 */

#include "interface.h"
#include <stdint.h>
#ifndef INTERFACE_TIMER_H
#define INTERFACE_TIMER_H

void interface_timer_set_arr(hal_abc_ptr_t handle, uint32_t arr);
void interface_timer_set_psc(hal_abc_ptr_t handle, uint32_t psc);

#endif