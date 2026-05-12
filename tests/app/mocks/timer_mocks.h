#ifndef MOCK_TIMER_H
#define MOCK_TIMER_H

#include "interface_timer.h"
#include <stdint.h>

typedef struct
{
    int change_psc_called;
    int change_arr_called;
    uint32_t last_psc;
    uint32_t last_arr;
    hal_abc_ptr_t last_handle;
} timer_mock_state_t;

extern timer_mock_state_t g_timer_mock_state;

void mock_timer_change_psc(hal_abc_ptr_t handle, uint32_t psc);
void mock_timer_change_arr(hal_abc_ptr_t handle, uint32_t arr);
void mock_timer_reset(void);



#endif /* MOCK_TIMER_H */
