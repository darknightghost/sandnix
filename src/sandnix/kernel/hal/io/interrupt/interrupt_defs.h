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
#if defined X86
    #include "arch/x86/interrupt_defs.h"
#elif defined ARM_ARMV7_CORTEXA9
    #include "arch/arm/armv7/cortex-a9/interrupt_defs.h"
#endif

#include "../../cpu/cpu_defs.h"

#define	TICK_PERIOD		10000

struct	_context;
typedef	struct	_context	context_t, *pcontext_t;

//void	int_callback(u32 int_num, pcontext_t p_context, u32 err_code);
typedef void	(*int_callback_t)(u32, pcontext_t, u32);
