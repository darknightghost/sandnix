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

#include "../../common/common.h"
#include "../debug/debug.h"
#include "parameters/parameters.h"
#include "../exceptions/exceptions.h"
#include "../io/io.h"
#include "../mm/mm.h"
#include "../pm/pm.h"


void test();

void kernel_main()
{
	dbg_cls();
	io_set_crrnt_int_level(INT_LEVEL_DISPATCH);

	dbg_print("%s", "Sandnix 0.0.1 kernel loaded.\n");

	//Initialize
	get_kernel_param();
	io_init();
	excpt_init();
	mm_init();
	pm_init();


	//Create daemon threads
	io_enable_interrupt();
	dbg_print("Creating interrupt dispatcher thread...\n");
	pm_create_thrd(io_dispatch_int, true, false, INT_LEVEL_EXCEPTION, NULL);

	//Create driver_init process
	test();

	//IDLE
	io_set_crrnt_int_level(INT_LEVEL_IDLE);

	while(1);

	return;
}

void test()
{
	u32 new_tbl;
	char* p;

	p = mm_virt_alloc(NULL, 4096, MEM_RESERVE | MEM_COMMIT | MEM_USER, PAGE_WRITEABLE);
	new_tbl = mm_pg_tbl_fork(0);
	mm_pg_tbl_switch(new_tbl);
	*p = 'a';
	*(p + 1) = '\0';
	mm_pg_tbl_switch(0);

	__asm__ __volatile__("nop\n\tnop\n\tnop\n\tnop\n\t");

}
