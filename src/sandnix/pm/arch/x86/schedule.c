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
                             ((level)+1)/(\
                                     (INT_LEVEL_USR_HIGHEST+1)%MAX_TIME_SLICE_NUM\
                                     ?(INT_LEVEL_USR_HIGHEST+1)/MAX_TIME_SLICE_NUM+1\
                                     :(INT_LEVEL_USR_HIGHEST+1)/MAX_TIME_SLICE_NUM\
                                         )+1\
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
static	void			adjust_int_level();
static	void			user_thread_caller(u32 thread_id, void* p_args);
static	void			thread_recycler(u32 thread_id, void* p_args);

static	task_queue		task_queues[INT_LEVEL_HIGHEST + 1];
static	spin_lock		cpu0_schedule_lock;
static	u32				recycler_thread_id;

static	plist_node		join_list;
static	spin_lock		join_list_lock;

/*
	It's safe to edit the information of a thread in thread_table if you'v got
	the lock of the task queue which the thread is in.Because when a thread is
	TASK_SLEEP or TASK_READY or TASK_RUNNING,It must been removed from the task
	queue before it can be removed from thread_table.YOU SHOULD NOT GET
	thread_table_lock AFTER YOU'V GOT THE LOCK OF ANY TASK QUEUE.That may cause
	deadlock.
*/

//Ticks
static	u32				cpu0_tick;

void init_schedule()
{
	u32 i;
	plist_node p_new_node;

	cpu0_tick = 0;

	//Initialize thread table
	dbg_print("\nInitializing schedule...\n");
	dbg_print("Creating thread 0...\n");
	rtl_memset(thread_table, 0, sizeof(thread_table));
	thread_table[0].alloc_flag = true;
	thread_table[0].level = INT_LEVEL_USR_HIGHEST;
	thread_table[0].status_info.ready.time_slice = 0;
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
	pm_init_spn_lock(&cpu0_schedule_lock);
	schedule_heap = mm_hp_create(TASK_QUEUE_HEAP_SIZE, HEAP_MULTITHREAD);

	if(schedule_heap == NULL) {
		excpt_panic(EXCEPTION_ILLEGAL_HEAP_ADDR,
		            "Unable to create schedule heap!\n");
	}

	for(i = 0; i <= INT_LEVEL_HIGHEST; i++) {
		task_queues[i].queue = NULL;
		pm_init_spn_lock(&(task_queues[i].lock));
	}

	//Create thread 0
	dbg_print("Creating thread 0...\n");
	p_new_node = rtl_list_insert_after(
	                 &(task_queues[INT_LEVEL_USR_HIGHEST].queue),
	                 NULL,
	                 &thread_table[0],
	                 schedule_heap);

	if(p_new_node == NULL) {
		excpt_panic(EXCEPTION_UNKNOW,
		            "Unable to initialize task queue!\n");
	}

	thread_table[0].p_task_queue_node = p_new_node;

	//Create recycler thread
	pm_init_spn_lock(&join_list_lock);
	join_list = NULL;
	dbg_print("Creating stack recycler thread...\n");
	recycler_thread_id = pm_create_thrd(thread_recycler, true, false, NULL);

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
	    "call	pm_task_schedule\n\t"
	    "_ret:\n\t"
	    ::"i"(SELECTOR_K_CODE));
	return;
}

void pm_clock_schedule()
{
	u32 int_level;
	cpu0_tick++;

	int_level = io_get_crrnt_int_level();

	if(int_level <= INT_LEVEL_USR_HIGHEST) {
		if(cpu0_tick > TIME_SLICE_TICKS) {
			__asm__ __volatile__(
			    "leave\n\t"
			    "addl	$4,%%esp\n\t"
			    "call	pm_task_schedule\n\t"
			    ::);
		}

	} else {
		cpu0_tick = 0;
	}

	__asm__ __volatile__(
	    "leave\n\t"
	    "addl	$4,%%esp\n\t"
	    "popal\n\t"
	    "iret\n\t"
	    ::);

}

void pm_task_schedule()
{

	if(pm_try_acqr_raw_spn_lock(&cpu0_schedule_lock)) {
		adjust_int_level();
		cpu0_tick = 0;

		__asm__ __volatile__(""::"i"(get_next_task));
		__asm__ __volatile__(""::"i"(switch_to));
		__asm__ __volatile__(
		    "leave\n\t"
		    "addl	$4,%%esp\n\t"
		    "call	get_next_task\n\t"
		    "pushl	%%eax\n\t"
		    "call	switch_to\n\t"
		    ::);
	}

	__asm__ __volatile__(
	    "leave\n\t"
	    "addl	$4,%%esp\n\t"
	    "popal\n\t"
	    "iret\n\t"
	    ::);

}

void adjust_int_level()
{
	u32 int_level;
	u32 old_level;
	plist_node p_new_node;

	pm_acqr_raw_spn_lock(&thread_table_lock);
	int_level = io_get_crrnt_int_level();
	old_level = thread_table[current_thread].level;

	//Check if the interrupt level of current thread in the thread table is
	//equal to current interrupt level
	if(int_level != old_level) {

		//If current thread was not been put in the correct task queue
		pm_acqr_raw_spn_lock(&(task_queues[old_level].lock));
		rtl_list_remove(&(task_queues[old_level].queue),
		                thread_table[current_thread].p_task_queue_node,
		                schedule_heap);
		pm_rls_raw_spn_lock(&(task_queues[old_level].lock));

		pm_acqr_raw_spn_lock(&(task_queues[int_level].lock));

		if(thread_table[current_thread].status_info.ready.time_slice > 0) {
			p_new_node = rtl_list_insert_before(
			                 &(task_queues[int_level].queue),
			                 NULL,
			                 &thread_table[current_thread],
			                 schedule_heap);

		} else {
			p_new_node = rtl_list_insert_after(
			                 &(task_queues[int_level].queue),
			                 NULL,
			                 &thread_table[current_thread],
			                 schedule_heap);
		}

		if(p_new_node == NULL) {
			excpt_panic(EXCEPTION_RESOURCE_DEPLETED,
			            "Failes to append new item to task queue!\n");
		}

		//Set the interrupt level in the thread table
		thread_table[current_thread].level = int_level;
		thread_table[current_thread].p_task_queue_node = p_new_node;

		pm_rls_raw_spn_lock(&(task_queues[int_level].lock));

	}

	pm_rls_raw_spn_lock(&thread_table_lock);

	return;
}

u32 pm_create_thrd(thread_func entry,
                   bool is_ready,
                   bool	is_user,
                   void* p_args)
{
	void* k_stack;
	u32 new_id;
	u8* p_stack;
	pusr_thread_info p_usr_thread_info;
	ret_regs regs;

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

	//Returning address
	p_stack -= 4;

	//Prepare for iret
	//Eflags
	p_stack -= 4;
	*(u32*)p_stack = 0x200;

	//CS
	p_stack -= 4;
	*(u16*)p_stack = SELECTOR_K_CODE;

	//EIP
	p_stack -= 4;

	if(is_user) {
		*(thread_func*)p_stack = user_thread_caller;

	} else {
		*(thread_func*)p_stack = entry;
	}

	//pushal
	rtl_memset(&regs, 0, sizeof(ret_regs));
	regs.ebp = (u32)((u8*)k_stack + KERNEL_STACK_SIZE);
	regs.esp = (u32)p_stack;

	p_stack -= sizeof(ret_regs);
	rtl_memcpy(p_stack, &regs, sizeof(ret_regs));

	//Set stack
	thread_table[new_id].kernel_stack = k_stack;
	thread_table[new_id].ebp = (u32)((u8*)k_stack + KERNEL_STACK_SIZE);
	thread_table[new_id].esp = (u32)p_stack;

	thread_table[new_id].parent_id = current_thread;

	pm_rls_spn_lock(&thread_table_lock);

	if(is_ready) {
		pm_resume_thrd(new_id);
	}

	return new_id;

}

void pm_terminate_thrd(u32 thread_id, u32 exit_code)
{
	u32 level;

	pm_acqr_spn_lock(&thread_table_lock);

	//Check the status of the thread
	if(thread_table[thread_id].alloc_flag == false
	   || thread_table[thread_id].status == TASK_ZOMBIE) {
		pm_rls_spn_lock(&thread_table_lock);
		return;
	}

	level = thread_table[thread_id].level;
	//Remove the thread from task queue
	pm_acqr_spn_lock(&(task_queues[level].lock));
	rtl_list_remove(
	    &(task_queues[level].queue),
	    thread_table[thread_id].p_task_queue_node,
	    schedule_heap);
	pm_rls_spn_lock(&(task_queues[level].lock));

	//Set thread status
	thread_table[thread_id].status = TASK_ZOMBIE;
	thread_table[thread_id].exit_code = exit_code;

	pm_rls_spn_lock(&thread_table_lock);

	pm_resume_thrd(recycler_thread_id);
	return;
}

void pm_suspend_thrd(u32 thread_id)
{
	u32 level;
	u32 current_level;

	current_level = io_get_crrnt_int_level();
	pm_acqr_spn_lock(&thread_table_lock);

	//Check the status of the thread
	if(thread_table[thread_id].alloc_flag == false
	   || (thread_table[thread_id].status != TASK_READY
	       && thread_table[thread_id].status != TASK_RUNNING
	       && thread_table[thread_id].status != TASK_SLEEP)) {
		pm_rls_spn_lock(&thread_table_lock);
		return;
	}

	//Remove the thread from task queue
	level = thread_table[thread_id].level;
	pm_acqr_spn_lock(&(task_queues[level].lock));
	rtl_list_remove(
	    &(task_queues[level].queue),
	    thread_table[thread_id].p_task_queue_node,
	    schedule_heap);
	pm_rls_spn_lock(&(task_queues[level].lock));
	thread_table[thread_id].status = TASK_SUSPEND;
	thread_table[thread_id].level = current_level;

	pm_rls_spn_lock(&thread_table_lock);
	pm_schedule();
	return;
}

void pm_resume_thrd(u32 thread_id)
{
	plist_node p_new_node;
	u32 int_level;

	pm_acqr_spn_lock(&thread_table_lock);

	if(thread_table[thread_id].alloc_flag == false
	   || thread_table[thread_id].status != TASK_SUSPEND) {
		pm_rls_spn_lock(&thread_table_lock);
		return;
	}

	int_level = thread_table[thread_id].level;

	//Set thread status and time slice
	thread_table[thread_id].status = TASK_READY;
	thread_table[thread_id].status_info.ready.time_slice
	    = TIME_SLICE(int_level);

	//Add thread to task queue
	pm_acqr_spn_lock(&(task_queues[int_level].lock));
	p_new_node = rtl_list_insert_before(
	                 &(task_queues[int_level].queue),
	                 NULL,
	                 &thread_table[thread_id],
	                 schedule_heap);

	if(p_new_node == NULL) {
		excpt_panic(EXCEPTION_RESOURCE_DEPLETED,
		            "Failes to append new item to task queue!\n");
	}

	thread_table[thread_id].p_task_queue_node = p_new_node;

	pm_rls_spn_lock(&(task_queues[int_level].lock));
	pm_rls_spn_lock(&thread_table_lock);

	pm_schedule();
	return;
}

void pm_sleep(u32 ms)
{
	u32 tick;

	pm_acqr_spn_lock(&thread_table_lock);
	thread_table[current_thread].status = TASK_SLEEP;
	tick = io_get_tick_count();
	thread_table[current_thread].status_info.sleep.start_tick = tick;
	thread_table[current_thread].status_info.sleep.stop_tick = tick + ms * 10 / SYS_TICK;
	pm_rls_spn_lock(&thread_table_lock);
	pm_schedule();
	return;
}

u32 pm_get_crrnt_thrd_id()
{
	return current_thread;
}

u32 pm_join(u32 thread_id)
{
	int i;

	while(1) {
		pm_acqr_spn_lock(&thread_table_lock);

		for(i = 0; i < MAX_THREAD_NUM; i++) {
			if((thread_id == 0
			    && thread_table[i].process_id == current_process)
			   || thread_id == i) {
				if(thread_table[i].alloc_flag
				   && thread_table[i].status == TASK_ZOMBIE) {
					thread_table[i].alloc_flag = false;
					return thread_table[i].exit_code;

				}
			}
		}

		pm_rls_spn_lock(&thread_table_lock);
		pm_suspend_thrd(current_thread);
	}

	return 0;
}

void switch_to(u32 thread_id)
{
	u32 proc_id;
	u32 old_esp;
	u32 old_ebp;
	u8* current_ebp;

	if(current_thread == thread_id) {
		pm_rls_raw_spn_lock(&cpu0_schedule_lock);
		__asm__ __volatile__(
		    "leave\n\t"
		    "addl	$8,%%esp\n\t"
		    "popal\n\t"
		    "iret\n\t"
		    ::);
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
	io_set_crrnt_int_level(thread_table[thread_id].level);

	//Save esp,ebp
	__asm__ __volatile__(
	    "movl	%%ebp,%0\n\t"
	    :"=r"(current_ebp)
	    :);
	old_esp = (u32)(current_ebp + 4 + 4 + 4);
	old_ebp = *(u32*)current_ebp;
	thread_table[current_thread].esp = old_esp;
	thread_table[current_thread].ebp = old_ebp;

	if(thread_table[current_thread].status == TASK_RUNNING) {
		thread_table[current_thread].status = TASK_READY;
	}

	current_thread = thread_id;
	current_process = proc_id;
	pm_rls_raw_spn_lock(&cpu0_schedule_lock);

	//Set esp,ebp
	__asm__ __volatile__(
	    "movl	(%0),%%eax\n\t"
	    "movl	%%eax,%%esp\n\t"
	    "movl	(%1),%%eax\n\t"
	    "movl	%%eax,%%ebp\n\t"
	    "popal\n\t"
	    "iret\n\t"
	    ::"b"(&(thread_table[thread_id].esp)),
	    "c"(&(thread_table[thread_id].ebp)));

	return;
}

u32 get_next_task()
{
	u32 i;
	plist_node p_node;
	plist_node p_new_node;
	pthread_info p_info;
	bool idle_flag;
	u32 current_tick;
	u32 ret_id;

	idle_flag = true;

	while(1) {
		for(i = INT_LEVEL_HIGHEST;
		    i + 1 > 0;
		    i--) {
			if(i > INT_LEVEL_USR_HIGHEST) {

				//Real-time thread
				//FCFS
				pm_acqr_raw_spn_lock(&(task_queues[i].lock));

				if(task_queues[i].queue != NULL) {

					//Call the highest-int-level thread
					p_node = task_queues[i].queue;

					do {
						p_info = (pthread_info)(p_node->p_item);
						current_tick = io_get_tick_count();

						if(p_info->status == TASK_READY
						   || p_info->status == TASK_RUNNING) {
							p_info->status = TASK_RUNNING;
							pm_rls_raw_spn_lock(&(task_queues[i].lock));
							ret_id = ((u32)p_info - (u32)thread_table) / sizeof(thread_info);
							return ret_id;

						} else if(p_info->status == TASK_SLEEP
						          && current_tick - p_info->status_info.sleep.start_tick
						          >= p_info->status_info.sleep.stop_tick
						          - p_info->status_info.sleep.start_tick) {

							//Awake sleeping thread
							p_info->status = TASK_READY;
							p_info->status_info.ready.time_slice = 0;
						}

						p_node = p_node->p_next;
					} while(p_node != task_queues[i].queue);
				}

				pm_rls_raw_spn_lock(&(task_queues[i].lock));

			} else if(i > INT_LEVEL_IDLE) {
				//Normal thread
				//Round Robin
				pm_acqr_raw_spn_lock(&(task_queues[i].lock));

				if(task_queues[i].queue != NULL) {

					//Look for TASK_READY thread
					p_node = task_queues[i].queue;

					do {
						p_info = (pthread_info)(p_node->p_item);
						current_tick = io_get_tick_count();

						if(p_info->status == TASK_READY
						   || p_info->status == TASK_RUNNING) {
							idle_flag = false;

							if(p_info->status_info.ready.time_slice > 0) {
								p_info->status = TASK_RUNNING;

								//Decrease time slice
								(p_info->status_info.ready.time_slice)--;

								if(p_info->status_info.ready.time_slice == 0) {
									rtl_list_remove(&(task_queues[i].queue),
									                p_node,
									                schedule_heap);

									p_new_node = rtl_list_insert_after(
									                 &(task_queues[i].queue),
									                 NULL,
									                 p_info,
									                 schedule_heap);

									if(p_new_node == NULL) {
										excpt_panic(EXCEPTION_RESOURCE_DEPLETED,
										            "Failes to append new item to task queue!\n");
									}

									p_info->p_task_queue_node = p_new_node;
								}

								pm_rls_raw_spn_lock(&(task_queues[i].lock));
								ret_id = ((u32)p_info - (u32)thread_table) / sizeof(thread_info);
								return ret_id;
							}

						} else if(p_info->status == TASK_SLEEP
						          && current_tick - p_info->status_info.sleep.start_tick
						          >= p_info->status_info.sleep.stop_tick
						          - p_info->status_info.sleep.start_tick) {

							//Awake sleeping thread
							p_info->status = TASK_READY;
							p_info->status_info.ready.time_slice = 0;
						}


						p_node = p_node->p_next;
					} while(p_node != task_queues[i].queue);
				}

				pm_rls_raw_spn_lock(&(task_queues[i].lock));

			} else if(idle_flag) {
				//IDLE thread
				pm_acqr_raw_spn_lock(&(task_queues[INT_LEVEL_IDLE].lock));

				if(task_queues[INT_LEVEL_IDLE].queue != NULL) {

					//Look for TASK_READY thread
					p_node = task_queues[INT_LEVEL_IDLE].queue;

					do {
						p_info = (pthread_info)(p_node->p_item);
						current_tick = io_get_tick_count();

						if(p_info->status == TASK_READY
						   || p_info->status == TASK_RUNNING) {
							idle_flag = false;

							if(p_info->status_info.ready.time_slice < 0) {
								p_info->status = TASK_RUNNING;

								//Decrease time slice
								(p_info->status_info.ready.time_slice)--;

								if(p_info->status_info.ready.time_slice == 0) {
									rtl_list_remove(&(task_queues[i].queue),
									                p_node,
									                schedule_heap);

									p_new_node = rtl_list_insert_after(
									                 &(task_queues[i].queue),
									                 NULL,
									                 p_info,
									                 schedule_heap);

									if(p_new_node == NULL) {
										excpt_panic(EXCEPTION_RESOURCE_DEPLETED,
										            "Failes to append new item to task queue!\n");
									}

									p_info->p_task_queue_node = p_new_node;
								}

								pm_rls_raw_spn_lock(&(task_queues[INT_LEVEL_IDLE].lock));
								ret_id = ((u32)p_info - (u32)thread_table) / sizeof(thread_info);
								return ret_id;
							}

						} else if(p_info->status == TASK_SLEEP
						          && current_tick - p_info->status_info.sleep.start_tick
						          >= p_info->status_info.sleep.stop_tick
						          - p_info->status_info.sleep.start_tick) {

							//Awake sleeping thread
							p_info->status = TASK_READY;
							p_info->status_info.ready.time_slice = 0;
						}

						p_node = p_node->p_next;
					} while(p_node != task_queues[i].queue);
				}

				pm_rls_raw_spn_lock(&(task_queues[INT_LEVEL_IDLE].lock));
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
	pm_acqr_raw_spn_lock(&(task_queues[INT_LEVEL_IDLE].lock));
	p_node = task_queues[INT_LEVEL_IDLE].queue;

	if(p_node != NULL) {
		p_info = (pthread_info)(p_node->p_item);

		if(p_info->status_info.ready.time_slice == 0) {
			//Reset time slice
			do {
				p_info = (pthread_info)(p_node->p_item);
				p_info->status_info.ready.time_slice = 1;
				p_node = p_node->p_next;
			} while(p_node != task_queues[INT_LEVEL_IDLE].queue);
		}
	}

	pm_rls_raw_spn_lock(&(task_queues[INT_LEVEL_IDLE].lock));

	//Normal thread
	for(i = 1; i <= INT_LEVEL_USR_HIGHEST; i++) {
		pm_acqr_raw_spn_lock(&(task_queues[i].lock));
		p_node = task_queues[i].queue;

		if(p_node != NULL) {
			p_info = (pthread_info)(p_node->p_item);

			//Reset time slice
			do {
				p_info = (pthread_info)(p_node->p_item);
				p_info->status_info.ready.time_slice = TIME_SLICE(i);
				p_node = p_node->p_next;
			} while(p_node != task_queues[i].queue);
		}

		pm_rls_spn_lock(&(task_queues[i].lock));
	}

	return;
}

void thread_recycler(u32 thread_id, void* p_args)
{
	plist_node p_node;
	list current_join_list = NULL;

	io_set_crrnt_int_level(INT_LEVEL_DISPATCH);

	while(1) {
		pm_suspend_thrd(thread_id);

		//Copy the list
		pm_acqr_spn_lock(&join_list_lock);

		p_node = join_list;

		do {
			rtl_list_insert_after(&current_join_list, NULL,
			                      p_node->p_item, schedule_heap);
		} while(p_node != join_list);

		pm_rls_spn_lock(&join_list_lock);

		//Wake up all of the threads which are calling pm_join
		p_node = current_join_list;

		do {
			pm_resume_thrd((u32)(p_node->p_item));
		} while(p_node != join_list);

		rtl_list_destroy(&current_join_list, schedule_heap, NULL);

	}
}

void user_thread_caller(u32 thread_id, void* p_args)
{
	return;
}
