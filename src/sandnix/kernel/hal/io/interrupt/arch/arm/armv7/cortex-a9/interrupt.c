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

void swi_hndlr(pcontext_t p_context)
{
    hal_cpu_context_load(p_context);
}
