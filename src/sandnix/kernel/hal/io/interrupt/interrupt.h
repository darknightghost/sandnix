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

#pragma once

#include "../../../../../common/common.h"
#include "../../cpu/cpu.h"

//void	int_callback(u32 int_num, pcontext_t p_context, u32 err_code);
typedef void	(*int_callback_t)(u32, pcontext_t, u32);

//Initialize
void interrupt_init();

//Initialize cpu core
void interrupt_cpu_core_init(u32 cpuid);

//Release cpu core
void interrupt_cpu_core_release(u32 cpuid);

//Disable interrupt on current cpu
void hal_io_int_disable();

//Enable interrupt on current cpu
void hal_io_int_enable();

//Enable all IRQ
void hal_io_irq_enable_all();

//Disbale all IRQ
void hal_io_irq_disable_all();

//Enable IRQ
void hal_io_irq_enable(
    u32 num);		//Interrupt number of IRQ

//Disbale IRQ
void hal_io_irq_disable(
    u32 num);		//Interrupt number of IRQ

//Get IRQ range
void hal_io_get_irq_range(
    u32* p_begin,	//Begining IRQ
    u32 num);		//Number of IRQ

//Regist/unregist interrupt callback
void* hal_io_int_callback_reg(
    u32 num,					//Interrupt number
    int_callback_t callback);	//Callback

//Regist clock call back
void* hal_io_clock_callback_reg(
    u32 microsecond,
    int_callback_t callback);	//Callback

//Get system tick
u64 hal_io_get_ticks();

//Set system clock frequency
void hal_io_set_clock_freq(
    u32 freq);		//Frequency (Hz)

//Get system clock frequency
u32 hal_io_get_clock_freq();
