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

#include "../../backtrace.h"
#include "../../../../cpu/cpu.h"

#include "../../../../init/init.h"
#include "../../../../exception/exception.h"

#include "../../../../../core/mm/mm.h"
#include "../../../../../core/pm/pm.h"

void hal_debug_backtrace(plist_t p_ret, pcontext_t p_context, pheap_t heap)
{
    address_t ebp;

    //Initialize return list
    core_rtl_list_init(p_ret);

    //Get stack base
    ebp = p_context->ebp;

    address_t stack_base;
    size_t stack_size;

    core_pm_get_currnt_stack_info(&stack_base, &stack_size);

    stack_base += stack_size;

    //Backtrace
    while(ebp < stack_base) {
        address_t new_ebp = *(u32*)ebp;
        ebp += 4;
        void* func_addr = *(void**)ebp;
        ebp = new_ebp;
        core_rtl_list_insert_after(NULL, p_ret, func_addr, heap);
    }

    return;
}
