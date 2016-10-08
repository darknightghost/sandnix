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

#define	hal_cpu_context_load(p_context)	{ \
        __asm__ __volatile__( \
                              "movl		%0, %%esp\n" \
                              /*Prepare for iret*/ \
                              /*SS*/ \
                              "pushl	(108 + 4 * 4)(%%ebx)\n" \
                              /*ESP*/ \
                              "pushl	(108 - 5 * 4)(%0)\n" \
                              /*EFLAGS*/ \
                              "pushl	(108 + 5 * 4)(%0)\n" \
                              /*CS*/ \
                              "pushl	(108 + 6 * 4)(%0)\n" \
                              /*EIP*/ \
                              "pushl	(108 + 7 * 4)(%0)\n" \
                              \
                              /*Load context*/ \
                              "movl		%0, %%esp\n" \
                              "frstor	(%%esp)\n" \
                              "addl		$108, %%esp\n" \
                              "popl		%%eax\n" \
                              "movw		%%ax, %%es\n" \
                              "popl		%%eax\n" \
                              "movw		%%ax, %%ds\n" \
                              "popl		%%eax\n" \
                              "movw		%%ax, %%fs\n" \
                              "popl		%%eax\n" \
                              "movw		%%ax, %%gs\n" \
                              "addl		$(4 * 4), %%esp\n" \
                              "popal\n" \
                              \
                              /*iret*/ \
                              "subl		%1, %%esp\n" \
                              "" \
                              "" \
                              ::"b"(p_context), \
                              "i"(sizeof(context_t) + 5 * 4) \
                              :"memory"); \
    }

#define hal_cpu_context_save_call(dest_func)

#define	hal_cpu_get_stack_base(buff, size)	((void*)((address_t)(buff) + (size)))
