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

#if defined X86
    #include "arch/x86/interrupt.h"
#elif defined ARM_ARMV7_CORTEXA9
    #include "arch/arm/armv7/cortex-a9/interrupt.h"
#endif

#define	TICK_PERIOD		10000

//void	int_callback(u32 int_num, pcontext_t p_context, u32 err_code);
typedef void	(*int_callback_t)(u32, pcontext_t, u32);

//Initialize
void interrupt_init();

//Initialize cpu core
void interrupt_cpu_core_init(u32 cpuid);

//Release cpu core
void interrupt_cpu_core_release(u32 cpuid);

//Set kernel stack
void hal_io_set_krnl_stack(address_t addr);

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
    u32* p_num);	//Number of IRQ

//Send EOI
void hal_io_irq_send_eoi();

//Regist/unregist interrupt callback
void* hal_io_int_callback_set(
    u32 num,					//Interrupt number
    int_callback_t callback);	//Callback

//Get system tick
u64 hal_io_get_ticks();

//Set system clock period
void hal_io_set_clock_period(
    u32 microsecond);

//Get system clock period
u32 hal_io_get_clock_period();

//Send IPI
void hal_io_send_IPI();

//Send EOI
void hal_io_IPI_send_eoi();
