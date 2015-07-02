/*
	Copyright 2015,暗夜幽灵 <darknightghost.cn@gmail.com>

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

#include "io/stdout.h"
#include "interrupt/interrupt.h"
#include "memory/memory.h"
#include "menu/menu.h"

void loader_main()
{
	setup_interrupt();
	cls(BG_BLACK | FG_BRIGHT_WHITE);
	print_string(
	    GET_REAL_ADDR("Protect mode entered.\n"),
	    FG_BRIGHT_WHITE | BG_BLACK,
	    BG_BLACK);
	init_heap();

	show_menu();

	return;
}
