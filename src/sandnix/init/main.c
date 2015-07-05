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


void test();

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
	io_enable_interrupt();
	test();

	//io_dispatch_int();

	while(1);

	return;
}

void test_thread(u32 thread_id, void* p_args)
{
	io_set_crrnt_int_level(1);
	u32 i;

	dbg_print("Task 1\n");

	while(1) {
		for(i = 0; i < 10000; i++);

		//dbg_print("1");
		__asm__ __volatile__(
		    "sti\n\t"
		    ::);
	}

	while(1);
}

void test()
{
	u32 i;
	io_set_crrnt_int_level(INT_LEVEL_USR_HIGHEST);
	pm_create_thrd(test_thread, true, false, NULL);

	dbg_print("Task 0\n");

	while(1) {
		for(i = 0; i < 10000; i++);

		//dbg_print("0");
		__asm__ __volatile__(
		    "sti\n\t"
		    ::);
	}

	//pm_suspend_thrd(0);
}
