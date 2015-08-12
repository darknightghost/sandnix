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
	io_set_crrnt_int_level(INT_LEVEL_USR_HIGHEST);

	//Create driver_init process
	test();

	//IDLE
	io_set_crrnt_int_level(INT_LEVEL_IDLE);

	while(1);

	return;
}

void test()
{
	u32 pid;
	char* p;
	u32 i;
	u32 exit_code = 10;

	io_set_crrnt_int_level(INT_LEVEL_USR_HIGHEST / 5);

	p = mm_virt_alloc(NULL, 4096, MEM_USER | MEM_COMMIT | MEM_RESERVE, PAGE_WRITEABLE);
	rtl_strcpy_s(p, 4096, "123456789");

	pid = pm_fork();

	if(pid > 0) {
		dbg_print("I'm parent!,my child is %d!\n", pid);
		i = 0;

		while(i < 4) {
			dbg_print("Parent p=\"%s\",\n", p);
			i++;
		}

		exit_code = 0;

		dbg_print("Waiting...\n");
		exit_code = pm_wait(0, true);
		dbg_print("Exit code is %u\n", exit_code);

		while(1);


	} else if(pid == 0) {
		extern u32 current_process;
		dbg_print("I'm child!I'm %u.\n", current_process);
		rtl_strcpy_s(p, 4096, "abcdefg");
		dbg_print("Child changed!\n", pid);
		i = 0;

		while(i < 10) {
			dbg_print("Child p=\"%s\",\n", p);
			i++;
		}

		dbg_print("Child exiting,exitcode = %u.\n", 10);
		pm_exit_thrd(exit_code);

	}

	else {
		dbg_print("Failed!\n");
	}

	__asm__ __volatile__("nop\n\tnop\n\tnop\n\tnop\n\t");

}
