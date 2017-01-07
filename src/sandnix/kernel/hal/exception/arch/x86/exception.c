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

#include "../../../../core/rtl/rtl.h"

#include "../../exception.h"
#include "../exception.h"
#include "../../../io/io.h"

void hal_exception_arch_init()
{
}

void die()
{
    hal_io_int_disable();

    while(true) {
        __asm__ __volatile__("hlt\n");
    }

    return;
}
void do_raise(pexcept_obj_t p_exception, char* file, u32 line, char* comment,
              pcontext_t p_context)
{
    p_exception->raise(p_exception, p_context, file, line, comment);
    return;
}
