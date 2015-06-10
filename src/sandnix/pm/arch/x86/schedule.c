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

#include "../../pm.h"
#include "../../../rtl/rtl.h"
#include "schedule.h"
#include "../../../io/io.h"
#include "../../../setup/setup.h"
#include "../../../debug/debug.h"

tss		sys_tss;

static	u32				current_thread;
static	thread_info		thread_table[MAX_PROCESS_NUM];

static	void			switch_to(u32 thread_id);

void init_schedule()
{
	//Initialize thread table
	dbg_print("Initializing schedule...\n");
	rtl_memset(thread_table, 0, sizeof(thread_table));
	thread_table[0].alloc_flag = true;
	__asm__ __volatile__(
	    "movl	%%ebp,(%0)\n\t"
	    ::"r"(&(thread_table[0].esp0)));

	//Initialize schedule queue
	return;
}

void pm_schedule()
{
	__asm__ __volatile__(
	    "pushfl\n\t"
	    "lcalll		%0,$_call_schedule\n\t"
	    "_call_schedule:\n\t"
	    "popl	%%eax\n\t"
	    "pushl	$_ret\n\t"
	    "pushal\n\t"
	    "call	pm_task_schedule\n\t"
	    "_ret:\n\t"
	    ::"i"(SELECTOR_K_CODE));
	return;
}


void pm_suspend_thrd(u32 thread_id)
{

}

void pm_resume_thrd(u32 thread_id)
{

}

void pm_sleep(u32 ms)
{

}

u32 pm_get_crrnt_thrd_id()
{
	return 0;
}

void pm_task_schedule()
{
	io_dispatch_int();
}


void switch_to(u32 thread_id)
{
	return;
}
