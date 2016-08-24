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

#include "../../../../../../common/common.h"
#include "../../context/context.h"

#define	hal_cpu_context_load(p_context)	{ \
        if((p_context)->ss == SELECTOR_K_DATA) { \
            /* Return to kernel memory*/ \
            /* EIP */ \
            *((u32*)((p_context)->esp) - 3) = (p_context)->eip; \
            /* CS */ \
            *((u32*)((p_context)->esp) - 2) = (p_context)->cs; \
            /* EFLAGS */ \
            *((u32*)((p_context)->esp) - 1) = (p_context)->eflags; \
            (p_context)->esp = (p_context)->esp - 12; \
        } else{ \
            /* Return to user memory */ \
            /* EIP */ \
            *((u32*)(p_context) + sizeof(context_t) / 4 + 1) = (p_context)->eip; \
            /* EFLAGS */ \
            *((u32*)(p_context) + sizeof(context_t) / 4 + 3) = (p_context)->eflags; \
            /* ESP */ \
            *((u32*)(p_context) + sizeof(context_t) / 4 + 4) = (p_context)->esp; \
            (p_context)->esp = (u32)((u32*)(p_context) + sizeof(context_t) / 4 + 1); \
        } \
        __asm__ __volatile__( \
                              "movl		%0, %%esp\n" \
                              "frstor	(%%esp)\n" \
                              "addl		%1, %%esp\n" \
                              "addl		$32, %%esp\n" \
                              "iret\n" \
                              ::"r"(p_context), \
                              "i"(sizeof(fpu_env_t))); \
    }

u32		hal_cpu_get_cpuid();
