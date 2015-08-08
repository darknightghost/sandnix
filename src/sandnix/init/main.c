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
	int j;
	int *a;

	io_set_crrnt_int_level(INT_LEVEL_USR_HIGHEST / 5);

	p = mm_virt_alloc(NULL, 4096, MEM_USER | MEM_COMMIT | MEM_RESERVE, PAGE_WRITEABLE);
	rtl_strcpy_s(p, 4096, "123456789");
	a = (int*)(p + 10);

	pid = pm_fork();

	if(pid > 0) {
		dbg_print("I'm parent!,my child is %d!\n", pid);
		*a = 0;

		while(1) {
			for(j = 0; j < 10000000; j++);

			if(*a < 10) {
				dbg_print("Parent p=\"%s\",\n", p);
				(*a)++;
			}
		}


	} else if(pid == 0) {
		dbg_print("I'm child!\n");
		rtl_strcpy_s(p, 4096, "abcdefg");
		dbg_print("Child changed!\n", pid);
		*a = 0;

		while(1) {
			for(j = 0; j < 10000000; j++);

			if(*a < 10) {
				dbg_print("Child p=\"%s\",\n", p);
				(*a)++;
			}
		}

	}

	else {
		dbg_print("Failed!\n");
	}

	__asm__ __volatile__("nop\n\tnop\n\tnop\n\tnop\n\t");

}
