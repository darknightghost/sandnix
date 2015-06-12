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

#include "../common/common.h"
#include "../debug/debug.h"
#include "parameters/parameters.h"
#include "../exceptions/exceptions.h"
#include "../io/io.h"
#include "../mm/mm.h"
#include "../pm/pm.h"

void kernel_main()
{
	dbg_cls();
	io_set_crrnt_int_level(INT_LEVEL_DISPATCH);

	dbg_print("%s", "Sandnix 0.0.1 kernel loaded.\n");

	get_kernel_param();
	io_init();
	excpt_init();
	mm_init();
	pm_init();

	while(1);

	io_dispatch_int();

	return;
}
