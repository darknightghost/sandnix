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
#include "../../../../common/common.h"

#if defined X86
    #include "./arch/x86/early_print.h"
#endif

//Initialize
void hal_early_print_init();

//Clear the screen
void hal_early_print_cls();

//Switch color
void hal_early_print_color(u32 fg, u32 bg);

//Print string
void hal_early_print_puts(char* str);
