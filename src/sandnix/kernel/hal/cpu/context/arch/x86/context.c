/*
    Copyright 2017,王思远 <darknightghost.cn@gmail.com>

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

#include "../../../../../../../common/common.h"
#include "./context_defs.h"
#include "../../../../init/init.h"
#include "../../../../../core/rtl/rtl.h"
#include "../../../../../core/pm/pm.h"

static	void		thread_caller(void* entry, u32 thread_id, void* p_args);

pcontext_t hal_cpu_get_init_kernel_context(void* k_stack,
        size_t k_stack_size, void* entry,
        u32 thread_id, void* p_args)
{
    //Get stack base
    u32* stack_base = (u32*)((address_t)k_stack + k_stack_size);

    //Push parameters
    //p_args
    stack_base--;
    *stack_base = (u32)p_args;

    //thread_id
    stack_base--;
    *stack_base = thread_id;

    //entry
    stack_base--;
    *stack_base = (u32)entry;

    //Push returning address
    stack_base--;
    *stack_base = (u32)0;

    address_t esp = (address_t)stack_base;

    //Get context address
    stack_base = (u32*)((address_t)stack_base - sizeof(context_t));

    if((address_t)stack_base % 8 != 0) {
        stack_base = (u32*)((address_t)stack_base - (address_t)stack_base % 8);
    }

    pcontext_t p_ret = (pcontext_t)stack_base;

    //Initialize context
    core_rtl_memset(p_ret, 0, sizeof(context_t));
    p_ret->es = SELECTOR_K_DATA;
    p_ret->ds = SELECTOR_K_DATA;
    p_ret->fs = SELECTOR_K_DATA;
    p_ret->gs = SELECTOR_K_DATA;
    p_ret->ss = SELECTOR_K_DATA;
    p_ret->eflags = 0x00000200;
    p_ret->cs = SELECTOR_K_CODE;
    p_ret->eip = (u32)thread_caller;
    p_ret->edi = 0;
    p_ret->esi = 0;
    p_ret->ebp = ((address_t)k_stack + k_stack_size);
    p_ret->esp = esp;
    p_ret->ebx = 0;
    p_ret->edx = 0;
    p_ret->ecx = 0;
    p_ret->eax = 0;

    return p_ret;
}

void thread_caller(void* entry, u32 thread_id, void* p_args)
{
    void* retval = ((thread_func_t)entry)(thread_id, p_args);
    core_pm_exit(retval);
}
