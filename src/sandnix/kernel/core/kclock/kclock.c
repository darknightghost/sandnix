/*
    Copyright 2016,王思远 <darknightghost.cn@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "./kclock.h"
#include "../pm/pm.h"
#include "../../hal/io/io.h"
#include "../../hal/cpu/cpu.h"

static volatile	u64			microseconds;
static volatile	u32			interval;
static volatile	u32			next_interval;

static	void		on_clock(u32 int_num, pcontext_t p_context,
                             u32 err_code);

void core_kclock_init()
{
    //Initialize variables
    microseconds = 0;
    interval = INIT_CLOCK_INTERVAL;
    next_interval = INIT_CLOCK_INTERVAL;

    //Set interrupt handler
    hal_io_set_clock_period(INIT_CLOCK_INTERVAL);
    hal_io_int_callback_set(INT_CLOCK, on_clock);

    return;
}

u64 core_kclock_get_ms()
{
    return microseconds;
}

void core_kclock_set_interval(u32 microseconds)
{
    next_interval = microseconds;
    return;
}


void on_clock(u32 int_num, pcontext_t p_context, u32 err_code)
{
    microseconds += interval;

    if(next_interval != interval) {
        interval = next_interval;
    }

    hal_io_set_clock_period(interval);

    hal_io_irq_send_eoi();
    hal_cpu_context_load(p_context);
    UNREFERRED_PARAMETER(int_num);
    UNREFERRED_PARAMETER(err_code);
}
