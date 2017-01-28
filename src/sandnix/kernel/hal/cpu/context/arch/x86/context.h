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

#include "../../../../../../../common/common.h"
#include "./context_defs.h"
#include "../../../../init/init_defs.h"
#include "../../../../../core/rtl/rtl.h"
#include "../../../../mmu/mmu_defs.h"

/*
	ATTENTION!!! Before using this function, make sure there is enough free
	spaces to prepare for iret.
*/

#define	hal_cpu_context_load(p_context)	{ \
        pcontext_t	__tmp_p_context = (p_context); \
        if(__tmp_p_context->cs == SELECTOR_K_CODE) { \
            if((address_t)p_context + sizeof(context_t) + 3 * 4 \
               != (address_t)(__tmp_p_context->esp)) { \
                /* Copy context */ \
                pcontext_t p_new_base = (pcontext_t)((address_t)(__tmp_p_context->esp) \
                                                     - 3 * 4 - sizeof(context_t)); \
                core_rtl_memmove(p_new_base, \
                                 p_context, \
                                 sizeof(context_t)); \
                __tmp_p_context = p_new_base; \
            } \
            \
            /* Return to kernel memory */ \
            /* EIP */ \
            *((u32*)__tmp_p_context + sizeof(context_t) / 4 + 0) = __tmp_p_context->eip; \
            /* CS */ \
            *((u32*)__tmp_p_context + sizeof(context_t) / 4 + 1) = __tmp_p_context->cs; \
            /* EFLAGS */ \
            *((u32*)__tmp_p_context + sizeof(context_t) / 4 + 2) = __tmp_p_context->eflags; \
            \
        } else { \
            /* Return to user memory */ \
            /* EIP */ \
            *((u32*)__tmp_p_context + sizeof(context_t) / 4 + 0) = __tmp_p_context->eip; \
            /* CS */ \
            *((u32*)__tmp_p_context + sizeof(context_t) / 4 + 1) = __tmp_p_context->cs; \
            /* EFLAGS */ \
            *((u32*)__tmp_p_context + sizeof(context_t) / 4 + 2) = __tmp_p_context->eflags; \
            /* ESP */ \
            *((u32*)__tmp_p_context + sizeof(context_t) / 4 + 3) = __tmp_p_context->esp; \
            /* SS */ \
            *((u32*)__tmp_p_context + sizeof(context_t) / 4 + 4) = __tmp_p_context->ss; \
        } \
        \
        __asm__ __volatile__( \
                              "movl		%0, %%esp\n" \
                              "frstor	(%%esp)\n" \
                              "addl		%1, %%esp\n" \
                              "addl		$32, %%esp\n" \
                              "popal\n" \
                              "iret\n" \
                              ::"r"(__tmp_p_context), \
                              "i"(sizeof(fpu_env_t))); \
    }

#define hal_cpu_context_save_call(dest_func)

#define	hal_cpu_get_stack_base(buff, size)	((void*)((address_t)(buff) + (size)))

#define hal_cpu_is_kernel_context(p_context) ((p_context)->eip > (address_t)KERNEL_MEM_BASE)
