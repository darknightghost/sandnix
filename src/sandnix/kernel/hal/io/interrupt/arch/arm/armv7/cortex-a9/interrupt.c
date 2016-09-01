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


#include "interrupt.h"
#include "ivt.h"
#include "../../../../../../exception/exception.h"

void interrupt_init()
{
    ivt_init();
    return;
}

void int_dispatcher(u32 int_num, pcontext_t p_context)
{
    while(1);

    UNREFERRED_PARAMETER(int_num);
    UNREFERRED_PARAMETER(p_context);
}

void reset_hndlr()
{
    PANIC(ENOTSUP, "Reset.");
}

void undef_hndlr(pcontext_t p_context)
{
    p_context->pc += 4;
    hal_cpu_context_load(p_context);
}

void swi_hndlr(pcontext_t p_context)
{
    hal_cpu_context_load(p_context);
}

void prefetch_abort_hndlr(pcontext_t p_context)
{
    hal_cpu_context_load(p_context);
}

void data_abort_hndlr(pcontext_t p_context)
{
    hal_cpu_context_load(p_context);
}

void reserved_hndlr()
{
    PANIC(ENOTSUP,
          "Reserved interrupt called.");
    return;
}

void irq_hndlr(pcontext_t p_context)
{
    hal_cpu_context_load(p_context);
}

void fiq_hndlr(pcontext_t p_context)
{
    hal_cpu_context_load(p_context);
}
