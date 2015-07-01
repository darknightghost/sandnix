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

#include "../../../rtl/rtl.h"
#include "../../pm.h"
#include "schedule.h"
#include "../../../io/io.h"
#include "../../../setup/setup.h"
#include "../../../debug/debug.h"
#include "../../../mm/mm.h"
#include "../../../exceptions/exceptions.h"

#define	TIME_SLICE(level)	(\
                             ((level)/((level)/MAX_TIME_SLICE_NUM)\
                              >=MAX_TIME_SLICE_NUM)\
                             ?MAX_TIME_SLICE_NUM\
                             :(level)/((level)/MAX_TIME_SLICE_NUM)\
                          )

tss					sys_tss;

void*				schedule_heap;
u32					current_thread;
u32					current_process;
thread_info			thread_table[MAX_THREAD_NUM];
spin_lock			thread_table_lock;
u32					free_thread_id_num;

static	u32				get_next_task();
static	u32				get_free_thread_id();
static	void			reset_time_slice();
static	void			switch_to(u32 thread_id);
static	void			resume_thread(u32 thread_id);
static	void			suspend_thread(u32 thread_id);
static	void			user_thread_caller(u32 thread_id, void* p_args);

//Task queues
static	task_queue		task_queues[INT_LEVEL_HIGHEST + 1];

//Ticks
static	u32				cpu0_tick;

void init_schedule()
{
	u32 i;

	cpu0_tick = 0;

	//Initialize thread table
	dbg_print("\nInitializing schedule...\n");
	dbg_print("Creating thread 0...\n");
	rtl_memset(thread_table, 0, sizeof(thread_table));
	thread_table[0].alloc_flag = true;
	thread_table[0].level = INT_LEVEL_DISPATCH;
	thread_table[0].status = TASK_READY;
	__asm__ __volatile__(
	    "movl	%%ebp,(%0)\n\t"
	    ::"r"(&(thread_table[0].kernel_stack)));
	pm_init_spn_lock(&thread_table_lock);
	free_thread_id_num = MAX_THREAD_NUM - 1;

	//Initialize TSS
	dbg_print("Initializing TSS...\n");
	rtl_memset(&sys_tss, 0, sizeof(sys_tss));
	sys_tss.ss0 = SELECTOR_K_DATA;
	sys_tss.io_map_base_addr = sizeof(sys_tss);

	//Initialize task queue
	dbg_print("Initializing task queue...\n");
	current_thread = 0;
	current_process = 0;
	schedule_heap = mm_hp_create(TASK_QUEUE_HEAP_SIZE, HEAP_MULTITHREAD);

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
	//Prepare for iret and call pm_task_schedule
	__asm__ __volatile__(
	    "pushfl\n\t"
	    "cli\n\t"
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

void pm_clock_schedule()
{
	cpu0_tick++;

	if(cpu0_tick > TIME_SLICE_TICKS) {
		__asm__ __volatile__(
		    "leave\n\t"
		    "addl	$4,%%esp\n\t"
		    "call	pm_task_schedule\n\t"
		    ::);

	} else {
		__asm__ __volatile__(
		    "leave\n\t"
		    "addl	$4,%%esp\n\t"
		    "popal\n\t"
		    "iretl\n\t"
		    ::);
	}
}

void pm_task_schedule()
{
	u32 id;

	cpu0_tick = 0;
	id = get_next_task();

	switch_to(id);

	__asm__ __volatile__(
	    "leave\n\t"
	    "addl	$4,%%esp\n\t"
	    "popal\n\t"
	    "iretl\n\t"
	    ::);

}

u32 pm_create_thrd(thread_func entry,
                   bool is_ready,
                   bool	is_user,
                   void* p_args)
{
	void* k_stack;
	u32 new_id;
	u8* p_stack;
	u32 eflags;
	pusr_thread_info p_usr_thread_info;

	pm_acqr_spn_lock(&thread_table_lock);

	//Allocate a new id
	new_id = get_free_thread_id();

	if(new_id == 0) {
		pm_rls_spn_lock(&thread_table_lock);
		return 0;
	}

	//Allocate kernel stack
	k_stack = mm_virt_alloc(NULL, KERNEL_STACK_SIZE,
	                        MEM_RESERVE | MEM_COMMIT,
	                        PAGE_WRITEABLE);

	if(k_stack == NULL) {
		thread_table[new_id].alloc_flag = false;
		pm_rls_spn_lock(&thread_table_lock);
		return 0;
	}

	//Prepare kernel stack
	p_stack = (u8*)k_stack + KERNEL_STACK_SIZE;

	//Prepare parameters
	if(is_user) {
		p_stack -= sizeof(usr_thread_info);
		p_usr_thread_info = (pusr_thread_info)p_stack;
		p_usr_thread_info->func = entry;
		p_usr_thread_info->p_args = p_args;
		p_stack -= 4;
		*(pusr_thread_info*)p_stack = p_usr_thread_info;
		p_stack -= 4;
		*(u32*)p_stack = new_id;

	} else {
		p_stack -= 4;
		*(void**)p_stack = p_args;
		p_stack -= 4;
		*(u32*)p_stack = new_id;
	}

	//Prepare for iret
	p_stack -= 4;
	__asm__ __volatile__(
	    "pushfl\n\t"
	    "popl	%0\n\t"
	    :"=r"(eflags));
	*(u32*)p_stack = eflags;
	p_stack -= 2;
	*(u16*)p_stack = SELECTOR_K_DATA;
	p_stack -= 4;

	if(is_user) {
		*(thread_func*)p_stack = entry;

	} else {
		*(thread_func*)p_stack = user_thread_caller;
	}

	pm_rls_spn_lock(&thread_table_lock);

	if(is_ready) {
		pm_resume_thrd(new_id);
	}

	return new_id;

}

void pm_terminate_thrd(u32 thread_id, u32 exit_code);
void pm_suspend_thrd(u32 thread_id);
void pm_resume_thrd(u32 thread_id);
void pm_sleep(u32 ms);
u32 pm_get_crrnt_thrd_id();

void switch_to(u32 thread_id)
{
	u32 proc_id;

	if(current_thread == thread_id) {
		return;
	}

	proc_id = pm_get_proc_id(thread_id);

	//Switch page table
	if(proc_id != current_process) {
		pm_switch_process(proc_id);
	}

	//Set TSS
	sys_tss.esp0 = (u32)(thread_table[current_thread].kernel_stack);

	//Set MSR
	__asm__ __volatile__(
	    "movl	$0x0175,%%ecx\n\t"
	    "movl	%0,%%eax\n\t"
	    "wrmsr\n\t"
	    ::"m"(thread_table[current_thread].kernel_stack));

	//Save esp,ebp
	__asm__ __volatile__(
	    "movl	%%esp,(%0)\n\t"
	    "movl	%%ebp,(%1)\n\t"
	    ::"r"(&(thread_table[current_thread].esp)),
	    "r"(&(thread_table[current_thread].ebp)));

	if(thread_table[current_thread].status == TASK_RUNNING) {
		thread_table[current_thread].status = TASK_READY;
	}

	current_thread = thread_id;
	current_process = proc_id;

	//Set esp,ebp
	__asm__ __volatile__(
	    "movl	(%0),%%eax\n\t"
	    "movl	%%eax,%%esp\n\t"
	    "movl	(%1),%%eax\n\t"
	    "movl	%%eax,%%ebp\n\t"
	    "leave\n\t"
	    "ret\n\t"
	    ::"b"(&(thread_table[thread_id].esp)),
	    "c"(&(thread_table[thread_id].ebp)));

	return;
}

u32 get_next_task()
{
	u32 i;
	u32	old_int_level;
	plist_node p_node;
	pthread_info p_info;
	bool idle_flag;

	old_int_level = io_get_crrnt_int_level();
	io_set_crrnt_int_level(INT_LEVEL_DISPATCH);
	idle_flag = true;

	while(1) {
		for(i = INT_LEVEL_HIGHEST;
		    i + 1 > 0;
		    i--) {
			if(i > INT_LEVEL_USR_HIGHEST) {

				//Real-time thread
				//FCFS
				pm_acqr_spn_lock(&task_queues[i].lock);

				if(task_queues[i].queue != NULL) {

					//Call the highest-int-level thread
					p_node = task_queues[i].queue;

					do {
						p_info = (pthread_info)(p_node->p_item);

						if(p_info->status == TASK_READY) {
							p_info->status = TASK_RUNNING;
							pm_rls_spn_lock(&task_queues[i].lock);
							io_set_crrnt_int_level(old_int_level);
							return p_info - thread_table;
						}

						p_node = p_node->p_next;
					} while(p_node != task_queues[i].queue);
				}

				pm_rls_spn_lock(&task_queues[i].lock);

			} else if(i > INT_LEVEL_IDLE) {
				//Normal thread
				//Round Robin
				pm_acqr_spn_lock(&task_queues[i].lock);

				if(task_queues[i].queue != NULL) {

					//Look for TASK_READY thread
					p_node = task_queues[i].queue;

					do {
						p_info = (pthread_info)(p_node->p_item);

						if(p_info->status == TASK_READY) {
							idle_flag = false;

							if(p_info->status_info.ready.time_slice < 0) {
								p_info->status = TASK_RUNNING;

								//Decrease time slice
								(p_info->status_info.ready.time_slice)--;

								if(p_info->status_info.ready.time_slice == 0) {
									rtl_list_remove(&task_queues[i].queue,
									                p_node,
									                schedule_heap);

									if(rtl_list_insert_after(
									       &task_queues[i].queue,
									       NULL,
									       p_info,
									       schedule_heap) == NULL) {
										excpt_panic(EXCEPTION_RESOURCE_DEPLETED,
										            "Failes to append new item to task queue!\n");
									}
								}

								pm_rls_spn_lock(&task_queues[i].lock);
								io_set_crrnt_int_level(old_int_level);
								return p_info - thread_table;
							}
						}

						p_node = p_node->p_next;
					} while(p_node != task_queues[i].queue);
				}

				pm_rls_spn_lock(&task_queues[i].lock);

			} else if(idle_flag) {
				//IDLE thread
				pm_acqr_spn_lock(&task_queues[INT_LEVEL_IDLE].lock);

				if(task_queues[INT_LEVEL_IDLE].queue != NULL) {

					//Look for TASK_READY thread
					p_node = task_queues[INT_LEVEL_IDLE].queue;

					do {
						p_info = (pthread_info)(p_node->p_item);

						if(p_info->status == TASK_READY) {
							idle_flag = false;

							if(p_info->status_info.ready.time_slice < 0) {
								p_info->status = TASK_RUNNING;

								//Decrease time slice
								(p_info->status_info.ready.time_slice)--;

								if(p_info->status_info.ready.time_slice == 0) {
									rtl_list_remove(&task_queues[i].queue,
									                p_node,
									                schedule_heap);

									if(rtl_list_insert_after(
									       &task_queues[i].queue,
									       NULL,
									       p_info,
									       schedule_heap) == NULL) {
										excpt_panic(EXCEPTION_RESOURCE_DEPLETED,
										            "Failes to append new item to task queue!\n");
									}
								}

								pm_rls_spn_lock(&task_queues[INT_LEVEL_IDLE].lock);
								io_set_crrnt_int_level(old_int_level);
								return p_info - thread_table;
							}
						}

						p_node = p_node->p_next;
					} while(p_node != task_queues[i].queue);
				}

				pm_rls_spn_lock(&task_queues[INT_LEVEL_IDLE].lock);
			}
		}

		reset_time_slice();
	}
}

u32 get_free_thread_id()
{
	u32 i;

	for(i = 0; i < MAX_THREAD_NUM; i++) {
		if(!thread_table[i].alloc_flag) {
			rtl_memset(&(thread_table[i]), 0, sizeof(thread_table[i]));
			thread_table[i].alloc_flag = true;
			thread_table[i].status = TASK_SUSPEND;
			thread_table[i].level = INT_LEVEL_USR_HIGHEST;
			thread_table[i].process_id = current_process;
			free_thread_id_num--;
			return i;
		}
	}

	return 0;
}

void reset_time_slice()
{
	u32 i;
	plist_node p_node;
	pthread_info p_info;

	//IDLE threads
	pm_acqr_spn_lock(&(task_queues[INT_LEVEL_IDLE].lock));
	p_node = task_queues[INT_LEVEL_IDLE].queue;
	p_info = (pthread_info)(p_node->p_item);

	if(p_info->status_info.ready.time_slice == 0) {
		//Reset time slice
		do {
			p_info = (pthread_info)(p_node->p_item);
			p_info->status_info.ready.time_slice = 1;
			p_node = p_node->p_next;
		} while(p_node != task_queues[INT_LEVEL_IDLE].queue);
	}

	pm_rls_spn_lock(&(task_queues[INT_LEVEL_IDLE].lock));

	//Normal thread
	for(i = 1; i <= INT_LEVEL_USR_HIGHEST; i++) {
		p_node = task_queues[i].queue;
		p_info = (pthread_info)(p_node->p_item);

		//Reset time slice
		do {
			p_info = (pthread_info)(p_node->p_item);
			p_info->status_info.ready.time_slice = TIME_SLICE[i];
			p_node = p_node->p_next;
		} while(p_node != task_queues[i].queue);

	}

	return;
}

void resume_thread(u32 thread_id)
{
	return;
}

void suspend_thread(u32 thread_id)
{
	return;
}

void user_thread_caller(u32 thread_id, void* p_args)
{
	return;
}
