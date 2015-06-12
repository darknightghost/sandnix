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
#include "../../../mm/mm.h"
#include "../../../exceptions/exceptions.h"

tss		sys_tss;

static	void*			schedule_heap;
static	u32				current_thread;
static	u32				current_process;
static	thread_info		thread_table[MAX_PROCESS_NUM];
static	spin_lock		thread_table_lock;

static	u32				get_next_task(bool is_clock);
static	void			reset_time_slice();
static	bool			switch_to(u32 thread_id);

//Task queues
static	task_queue		task_queues[INT_LEVEL_HIGHEST + 1];

void init_schedule()
{
	u32 i;

	//Initialize thread table
	dbg_print("Initializing schedule...\n");
	rtl_memset(thread_table, 0, sizeof(thread_table));
	thread_table[0].alloc_flag = true;
	thread_table[0].level = INT_LEVEL_DISPATCH;
	thread_table[0].status = TASK_READY;
	__asm__ __volatile__(
	    "movl	%%ebp,(%0)\n\t"
	    ::"r"(&(thread_table[0].esp0)));
	pm_init_spn_lock(&thread_table_lock);

	//Initialize task queue
	current_thread = 0;
	current_process = 0;
	schedule_heap = mm_hp_create(1024, HEAP_EXTENDABLE);

	if(schedule_heap == NULL) {
		excpt_panic(EXCEPTION_ILLEGAL_HEAP_ADDR,
		            "Unable to create schedule heap!\n");
	}

	for(i = 0; i <= INT_LEVEL_HIGHEST; i++) {
		task_queues[i].queue = NULL;
		pm_init_spn_lock(&(task_queues[i].lock));
	}

	if(rtl_list_insert_after(
	       &(task_queues[INT_LEVEL_DISPATCH].queue),
	       NULL,
	       &thread_table[0],
	       schedule_heap) == NULL) {
		excpt_panic(EXCEPTION_UNKNOW,
		            "Unable to initialize task queue!\n");
	}

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
	    "pushl	$0\n\t"
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

void pm_task_schedule(bool is_clock)
{
	u32 id;

	__asm__ __volatile__(
	    "cli\n\t"
	    ::);

	id = get_next_task(is_clock);

	if(switch_to(id)) {
		//Reset tss
	}

	__asm__ __volatile__(
	    "leave\n\t"
	    "addl	$8,%%esp\n\t"
	    "iretd\n\t"
	    ::);

}


bool switch_to(u32 thread_id)
{
	u32 proc_id;

	if(current_thread == thread_id) {
		return false;
	}

	proc_id = pm_get_proc_id(thread_id);

	if(proc_id != current_process) {
		mm_pg_tbl_switch(pm_get_pdt_id(proc_id));
	}

	__asm__ __volatile__(
	    "movl	%%esp,(%0)\n\t"
	    "movl	%%ebp,(%1)\n\t"
	    ::"r"(&(thread_table[current_thread].esp0)),
	    "r"(&(thread_table[current_thread], ebp0)));
	current_thread = thread_id;
	current_process = proc_id;
	__asm__ __volatile__(
	    "movl	(%0),%%eax\n\t"
	    "movl	%%eax,%%esp\n\t"
	    "movl	(%1),%%eax\n\t"
	    "movl	%%eax,%%ebp\n\t"
	    "leave\n\t"
	    "movl	$1,%%eax\n\t"
	    "ret\n\t"
	    ::"b"(&(thread_table[thread_id].esp0)),
	    "c"(&(thread_table[thread_id], ebp0)));

	return;
}

u32 get_next_task(bool is_clock)
{
	return 0;
}

void reset_time_slice()
{

}
