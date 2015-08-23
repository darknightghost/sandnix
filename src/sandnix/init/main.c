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
#include "../rtl/rtl.h"
#include "../msg/msg.h"


void test();

void kernel_main()
{
	dbg_cls();

	dbg_print("%s", "Sandnix 0.0.1 kernel loaded.\n");

	io_set_crrnt_int_level(INT_LEVEL_DISPATCH);

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

	//Initialize
	msg_init();

	io_set_crrnt_int_level(INT_LEVEL_USR_HIGHEST);

	//Create driver_init process
	test();

	//IDLE
	io_set_crrnt_int_level(INT_LEVEL_IDLE);

	while(1);

	return;
}

void test_thread(u32 thread_id, void* p_args)
{
	u32 i, j;
	dbg_print("I'm thread %u.\n", thread_id);

	for(i = 0; i < 10; i++) {
		dbg_print("c\n");

		for(j = 0; j < 1000000; j++);
	}

	dbg_print("sqrt(10000) is %u.\n", (u32)rtl_sqrt(10000.0));

	pm_exit_thrd(1234);
	UNREFERRED_PARAMETER(p_args);
}

void test()
{
	u32 i, j;

	io_set_crrnt_int_level(INT_LEVEL_USR_HIGHEST / 5);
	pm_create_thrd(test_thread, true, false, INT_LEVEL_USR_HIGHEST / 5, NULL);

	for(i = 0; i < 10; i++) {
		dbg_print("p\n");

		for(j = 0; j < 1000000; j++);
	}

	dbg_print("Child thread exited with %u.\n", pm_join(0));

	__asm__ __volatile__("nop\n\tnop\n\tnop\n\tnop\n\t");

}
