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

tss_t					sys_tss;

void*				schedule_heap;
u32					current_thread = 0;
u32					current_process = 0;
thread_info_t			thread_table[MAX_THREAD_NUM];
spinlock_t			thread_table_lock;
u32					free_thread_id_num;

static	u32				get_next_task();
static	u32				get_free_thread_id();
static	void			reset_time_slice();
static	void			switch_to(u32 thread_id);
static	void			user_thread_caller(u32 thread_id, void* p_args);
static	void			join_thread();
static	void			adjust_child_thread_stack(u32 ebp, u32 offset);

static	task_queue_t		task_queues[INT_LEVEL_HIGHEST + 1];
static	spinlock_t		cpu0_schedule_lock;
static	bool			cpu0_task_switch_enabled = true;

/*
	It's safe to edit the information of a thread in thread_table if you'v got
	the lock of the task queue_t which the thread is in.Because when a thread is
	TASK_SLEEP or TASK_READY or TASK_RUNNING,It must been removed from the task
	queue_t before it can be removed from thread_table.YOU SHOULD NOT GET
	thread_table_lock AFTER YOU'V GOT THE LOCK OF ANY TASK QUEUE.That may cause
	deadlock.
*/

//Ticks
static	u32				cpu0_tick;

extern	char			init_stack[];

void init_schedule()
{
	u32 i;
	plist_node_t p_new_node;

	cpu0_tick = 0;

	//Initialize thread table
	dbg_print("\nInitializing schedule...\n");
	dbg_print("Creating thread 0...\n");
	rtl_memset(thread_table, 0, sizeof(thread_table));
	thread_table[0].alloc_flag = true;
	thread_table[0].level = INT_LEVEL_USR_HIGHEST;
	thread_table[0].status_info.ready.time_slice = 0;
	thread_table[0].status = TASK_READY;
	thread_table[0].kernel_stack = init_stack;
	pm_init_spn_lock(&thread_table_lock);
	free_thread_id_num = MAX_THREAD_NUM - 1;

	//Initialize TSS
	dbg_print("Initializing TSS...\n");
	rtl_memset(&sys_tss, 0, sizeof(sys_tss));
	sys_tss.ss0 = SELECTOR_K_DATA;
	sys_tss.io_map_base_addr = sizeof(sys_tss);

	//Initialize task queue_t
	dbg_print("Initializing task queue...\n");
	current_thread = 0;
	current_process = 0;
	pm_init_spn_lock(&cpu0_schedule_lock);
	schedule_heap = mm_hp_create(TASK_QUEUE_HEAP_SIZE, HEAP_MULTITHREAD);

	if(schedule_heap == NULL) {
		excpt_panic(EFAULT,
		            "Unable to create schedule heap!\n");
	}

	for(i = 0; i <= INT_LEVEL_HIGHEST; i++) {
		task_queues[i].queue_t = NULL;
		pm_init_spn_lock(&(task_queues[i].lock));
	}

	//Create thread 0
	dbg_print("Creating thread 0...\n");
	p_new_node = rtl_list_insert_after(
	                 &(task_queues[INT_LEVEL_USR_HIGHEST].queue_t),
	                 NULL,
	                 &thread_table[0],
	                 schedule_heap);

	if(p_new_node == NULL) {
		excpt_panic(EFAULT,
		            "Unable to initialize task queue!\n");
	}

	thread_table[0].p_task_queue_node = p_new_node;

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

	if(cpu0_task_switch_enabled) {
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
	plist_node_t p_new_node;

	pm_acqr_raw_spn_lock(&thread_table_lock);
	int_level = io_get_crrnt_int_level();
	old_level = thread_table[current_thread].level;

	//Check if the interrupt level of current thread in the thread table is
	//equal to current interrupt level
	if(int_level != old_level) {

		//If current thread was not been put in the correct task queue_t
		if(thread_table[current_thread].status == TASK_READY
		   || thread_table[current_thread].status == TASK_RUNNING
		   || thread_table[current_thread].status == TASK_SLEEP) {

			pm_acqr_raw_spn_lock(&(task_queues[old_level].lock));
			rtl_list_remove(&(task_queues[old_level].queue_t),
			                thread_table[current_thread].p_task_queue_node,
			                schedule_heap);
			pm_rls_raw_spn_lock(&(task_queues[old_level].lock));

			pm_acqr_raw_spn_lock(&(task_queues[int_level].lock));

			if(thread_table[current_thread].status_info.ready.time_slice > 0) {
				p_new_node = rtl_list_insert_before(
				                 &(task_queues[int_level].queue_t),
				                 NULL,
				                 &thread_table[current_thread],
				                 schedule_heap);

			} else {
				p_new_node = rtl_list_insert_after(
				                 &(task_queues[int_level].queue_t),
				                 NULL,
				                 &thread_table[current_thread],
				                 schedule_heap);
			}

			if(p_new_node == NULL) {
				excpt_panic(EFAULT,
				            "Failes to append new item to task queue_t!\n");
			}

		} else {
			pm_acqr_raw_spn_lock(&(task_queues[int_level].lock));
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
                   u32 priority,
                   void* p_args)
{
	void* k_stack;
	u32 new_id;
	u8* p_stack;
	pusr_thread_info_t p_usr_thread_info;
	ret_regs_t regs;

	pm_acqr_spn_lock(&process_table_lock);
	pm_acqr_spn_lock(&thread_table_lock);

	//Allocate a new id
	new_id = get_free_thread_id();

	if(new_id == 0) {
		pm_rls_spn_lock(&thread_table_lock);
		pm_rls_spn_lock(&process_table_lock);
		pm_set_errno(EAGAIN);
		return 0;
	}

	thread_table[new_id].level = priority;

	//Allocate kernel stack
	k_stack = mm_virt_alloc(NULL, KERNEL_STACK_SIZE,
	                        MEM_RESERVE | MEM_COMMIT,
	                        PAGE_WRITEABLE);

	if(k_stack == NULL) {
		thread_table[new_id].alloc_flag = false;
		pm_rls_spn_lock(&thread_table_lock);
		pm_rls_spn_lock(&process_table_lock);
		pm_set_errno(ENOMEM);
		return 0;
	}

	//Prepare kernel stack
	p_stack = (u8*)k_stack + KERNEL_STACK_SIZE;

	//Prepare parameters
	if(is_user) {
		p_stack -= sizeof(usr_thread_info_t);
		p_usr_thread_info = (pusr_thread_info_t)p_stack;
		p_usr_thread_info->func = entry;
		p_usr_thread_info->p_args = p_args;
		p_stack -= 4;
		*(pusr_thread_info_t*)p_stack = p_usr_thread_info;
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
	rtl_memset(&regs, 0, sizeof(ret_regs_t));
	regs.ebp = (u32)((u8*)k_stack + KERNEL_STACK_SIZE);
	regs.esp = (u32)p_stack;

	p_stack -= sizeof(ret_regs_t);
	rtl_memcpy(p_stack, &regs, sizeof(ret_regs_t));

	//Set stack
	thread_table[new_id].kernel_stack = k_stack;
	thread_table[new_id].ebp = (u32)((u8*)k_stack + KERNEL_STACK_SIZE);
	thread_table[new_id].esp = (u32)p_stack;

	//Initialize fpu
	rtl_memset(&(thread_table[new_id].fpu_data),
	           0,
	           sizeof(thread_table[new_id].fpu_data));
	thread_table[new_id].fpu_data.environment.control_word = 0x037F;
	thread_table[new_id].fpu_data.environment.tag_word = 0xFFFF;

	if(is_user) {
		add_proc_thrd(new_id, current_process);

	} else {
		add_proc_thrd(new_id, 0);
	}

	pm_rls_spn_lock(&thread_table_lock);
	pm_rls_spn_lock(&process_table_lock);

	if(is_ready) {
		pm_resume_thrd(new_id);
	}

	pm_set_errno(ESUCCESS);

	return new_id;

}

s32 fork_thread(u32 new_proc_id)
{
	void* k_stack;
	u32 new_id;
	u8* p_stack;
	ret_regs_t regs;
	u32	esp;
	u32 ebp;

	//Save registers
	__asm__ __volatile__(
	    "pushal\n\t"
	    "movl	%0,%%edi\n\t"
	    "movl	%%esp,%%esi\n\t"
	    "cld\n\t"
	    "movl	%1,%%ecx\n\t"
	    "rep	movsb\n\t"
	    "popal\n\t"
	    ::"a"(&regs), "i"(sizeof(regs)));

	pm_acqr_spn_lock(&thread_table_lock);

	//Allocate a new id
	new_id = get_free_thread_id();

	if(new_id == 0) {
		pm_rls_spn_lock(&thread_table_lock);
		pm_set_errno(EAGAIN);
		return -1;
	}

	//Fork thread info
	rtl_memcpy(&thread_table[new_id],
	           &thread_table[current_thread],
	           sizeof(thread_table[new_id]));
	thread_table[new_id].process_id = new_proc_id;
	thread_table[new_id].status = TASK_SUSPEND;

	//Allocate kernel stack_t
	k_stack = mm_virt_alloc(NULL, KERNEL_STACK_SIZE,
	                        MEM_RESERVE | MEM_COMMIT,
	                        PAGE_WRITEABLE);

	if(k_stack == NULL) {
		thread_table[new_id].alloc_flag = false;
		pm_rls_spn_lock(&thread_table_lock);
		pm_set_errno(ENOMEM);
		return -1;
	}

	//Copy kernel stack_t
	rtl_memcpy(k_stack,
	           thread_table[current_thread].kernel_stack,
	           KERNEL_STACK_SIZE);

	//Prepare kernel stack_t
	__asm__ __volatile__(
	    "movl	%%ebp,%0\n\t"
	    "movl	%%esp,%1\n\t"
	    :"=r"(ebp), "=r"(esp)
	    :);
	esp -= (u32)(thread_table[current_thread].kernel_stack);
	ebp -= (u32)(thread_table[current_thread].kernel_stack);
	esp += (u32)k_stack;
	ebp += (u32)k_stack;
	p_stack = (void*)esp;

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

	__asm__ __volatile__(
	    "jmp	_next\n\t"
	    "_child_ret:\n\t"
	    "leave\n\t"
	    "xorl	%%eax,%%eax\n\t"
	    "ret\n\t"
	    "_next:\n\t"
	    "movl	$_child_ret,(%0)\n\t"
	    ::"r"(p_stack):"memory");

	//pushal
	regs.ebp = ebp;
	regs.esp = (u32)p_stack;

	p_stack -= sizeof(ret_regs_t);
	rtl_memcpy(p_stack, &regs, sizeof(ret_regs_t));

	//Set stack_t
	thread_table[new_id].kernel_stack = k_stack;
	thread_table[new_id].ebp = ebp;
	thread_table[new_id].esp = (u32)p_stack;
	adjust_child_thread_stack(ebp,
	                          (u32)k_stack - (u32)(thread_table[current_thread].kernel_stack));

	pm_rls_spn_lock(&thread_table_lock);


	return new_id;

}

void pm_exit_thrd(u32 exit_code)
{
	u32 level;

	pm_acqr_spn_lock(&process_table_lock);
	pm_acqr_spn_lock(&thread_table_lock);

	//Check the status of the thread
	if(thread_table[current_thread].alloc_flag == false
	   || thread_table[current_thread].status == TASK_ZOMBIE) {
		pm_rls_spn_lock(&thread_table_lock);
		pm_rls_spn_lock(&process_table_lock);
		excpt_panic(ESRCH,
		            "Zombie thread running!");
		return;
	}

	level = thread_table[current_thread].level;

	thread_table[current_thread].exit_code = exit_code;
	set_exit_code(thread_table[current_thread].process_id, exit_code);

	//Remove the thread from task queue_t
	pm_disable_task_switch();
	pm_acqr_spn_lock(&(task_queues[level].lock));
	rtl_list_remove(
	    &(task_queues[level].queue_t),
	    thread_table[current_thread].p_task_queue_node,
	    schedule_heap);
	pm_rls_spn_lock(&(task_queues[level].lock));

	//Set thread status
	thread_table[current_thread].status = TASK_ZOMBIE;
	pm_rls_spn_lock(&thread_table_lock);

	zombie_proc_thrd(current_thread, current_process);
	pm_enable_task_switch();

	pm_rls_spn_lock(&process_table_lock);
	pm_schedule();

	excpt_panic(EFAULT,
	            "Zombie thread running after schedule!");

	return;
}

void pm_suspend_thrd(u32 thread_id)
{
	u32 level;
	u32 current_level;

	//Check arguments
	if(thread_id >= MAX_THREAD_NUM) {
		pm_set_errno(EOVERFLOW);
		return;
	}

	current_level = io_get_crrnt_int_level();
	pm_acqr_spn_lock(&thread_table_lock);

	//Check the status of the thread
	if(thread_table[thread_id].alloc_flag == false
	   || (thread_table[thread_id].status != TASK_READY
	       && thread_table[thread_id].status != TASK_RUNNING
	       && thread_table[thread_id].status != TASK_SLEEP)) {
		pm_rls_spn_lock(&thread_table_lock);
		pm_set_errno(ESRCH);
		return;
	}

	//Remove the thread from task queue_t
	level = thread_table[thread_id].level;

	pm_disable_task_switch();
	pm_acqr_spn_lock(&(task_queues[level].lock));
	rtl_list_remove(
	    &(task_queues[level].queue_t),
	    thread_table[thread_id].p_task_queue_node,
	    schedule_heap);
	pm_rls_spn_lock(&(task_queues[level].lock));
	thread_table[thread_id].status = TASK_SUSPEND;
	pm_enable_task_switch();

	thread_table[thread_id].level = current_level;

	pm_rls_spn_lock(&thread_table_lock);

	if(thread_id == current_thread) {
		pm_schedule();
	}

	pm_set_errno(ESUCCESS);

	return;
}

void pm_int_disaptch_suspend()
{
	u32 level;

	if(io_get_crrnt_int_level() < INT_LEVEL_EXCEPTION) {
		excpt_panic(EDEADLK,
		            "Function pm_int_disaptch_suspend() can be only used in thread io_dispatch_int().");
	}

	pm_acqr_raw_spn_lock(&thread_table_lock);

	//Remove the thread from task queue_t
	level = thread_table[current_thread].level;
	pm_disable_task_switch();
	pm_acqr_raw_spn_lock(&(task_queues[level].lock));
	rtl_list_remove(
	    &(task_queues[level].queue_t),
	    thread_table[current_thread].p_task_queue_node,
	    schedule_heap);
	pm_rls_raw_spn_lock(&(task_queues[level].lock));
	pm_enable_task_switch();
	thread_table[current_thread].status = TASK_SUSPEND;
	thread_table[current_thread].level = INT_LEVEL_EXCEPTION;

	pm_rls_raw_spn_lock(&thread_table_lock);

	pm_schedule();

	pm_set_errno(ESUCCESS);

	return;

}

void pm_resume_thrd(u32 thread_id)
{
	plist_node_t p_new_node;
	u32 int_level;

	//Check arguments
	if(thread_id >= MAX_THREAD_NUM) {
		pm_set_errno(EOVERFLOW);
		return;
	}

	pm_acqr_spn_lock(&thread_table_lock);

	if(thread_table[thread_id].alloc_flag == false)  {
		pm_rls_spn_lock(&thread_table_lock);
		pm_set_errno(ESRCH);
		return;
	}

	int_level = thread_table[thread_id].level;

	if(thread_table[thread_id].status == TASK_SLEEP) {
		//If the thread is sleeping
		thread_table[thread_id].status_info.ready.time_slice = TIME_SLICE(int_level);
		thread_table[thread_id].status = TASK_READY;
		pm_rls_spn_lock(&thread_table_lock);
		pm_set_errno(ESUCCESS);

		if(int_level > io_get_crrnt_int_level()) {
			pm_schedule();
		}

		return;

	} else if(thread_table[thread_id].status != TASK_SUSPEND
	          && thread_table[thread_id].status != TASK_WAIT
	          && thread_table[thread_id].status != TASK_JOIN) {
		pm_rls_spn_lock(&thread_table_lock);
		pm_set_errno(ESRCH);
		return;
	}

	//Set thread status and time slice
	thread_table[thread_id].status = TASK_READY;
	thread_table[thread_id].status_info.ready.time_slice
	    = TIME_SLICE(int_level);

	//Add thread to task queue_t
	pm_disable_task_switch();
	pm_acqr_spn_lock(&(task_queues[int_level].lock));
	p_new_node = rtl_list_insert_before(
	                 &(task_queues[int_level].queue_t),
	                 NULL,
	                 &thread_table[thread_id],
	                 schedule_heap);

	if(p_new_node == NULL) {
		excpt_panic(EFAULT,
		            "Failes to append new item to task queue_t!\n");
	}

	thread_table[thread_id].p_task_queue_node = p_new_node;

	pm_rls_spn_lock(&(task_queues[int_level].lock));
	pm_enable_task_switch();
	pm_rls_spn_lock(&thread_table_lock);

	if(int_level > io_get_crrnt_int_level()
	   && io_get_crrnt_int_level() <= INT_LEVEL_USR_HIGHEST) {
		pm_schedule();
	}

	pm_set_errno(ESUCCESS);

	return;
}

void pm_sleep(u32 ms)
{
	u32 tick;

	pm_acqr_spn_lock(&thread_table_lock);
	pm_disable_task_switch();
	thread_table[current_thread].status = TASK_SLEEP;
	tick = io_get_tick_count();
	thread_table[current_thread].status_info.sleep.start_tick = tick;
	thread_table[current_thread].status_info.sleep.stop_tick = tick + ms * 10 / SYS_TICK;
	pm_enable_task_switch();
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
	u32 proc_id;
	plist_node_t p_node;
	u32 exit_code;

	//Check arguments
	if(thread_id >= MAX_THREAD_NUM) {
		pm_set_errno(EOVERFLOW);
		return 0;
	}

	proc_id = thread_table[current_process].process_id;

	//Check the thread
	if(thread_id != 0) {
		pm_acqr_spn_lock(&thread_table_lock);

		if(thread_table[thread_id].alloc_flag == false
		   && thread_table[thread_id].process_id != proc_id) {
			pm_rls_spn_lock(&thread_table_lock);
			pm_set_errno(ESRCH);
			return 0;
		}

		pm_rls_spn_lock(&thread_table_lock);
	}

	//Join
	while(1) {
		pm_acqr_spn_lock(&process_table_lock);

		if(process_table[proc_id].zombie_list == NULL) {
			p_node = process_table[proc_id].thread_list;

			//If current thread is the only thread in current process
			if(p_node == p_node->p_next) {
				pm_rls_spn_lock(&process_table_lock);
				pm_set_errno(EDEADLK);
				return 0;
			}

		} else {
			if(thread_id == 0) {
				pm_acqr_spn_lock(&thread_table_lock);

				//Remove from zombie list_t
				p_node = process_table[proc_id].zombie_list;

				if(p_node != NULL) {
					//Free memory
					thread_id = (u32)(p_node->p_item);
					rtl_list_remove(&(process_table[proc_id].zombie_list),
					                p_node,
					                process_heap);
					mm_virt_free(thread_table[thread_id].kernel_stack,
					             KERNEL_STACK_SIZE,
					             MEM_UNCOMMIT | MEM_RELEASE);

					if(thread_table[thread_id].user_stack != NULL) {
						mm_virt_free(thread_table[thread_id].user_stack,
						             thread_table[thread_id].user_stack_size,
						             MEM_UNCOMMIT | MEM_RELEASE | MEM_USER);
					}

					exit_code = thread_table[thread_id].exit_code;
					pm_rls_spn_lock(&thread_table_lock);
					pm_rls_spn_lock(&process_table_lock);
					pm_set_errno(ESUCCESS);

					return exit_code;

				}

				pm_rls_spn_lock(&thread_table_lock);

			} else {
				pm_acqr_spn_lock(&thread_table_lock);

				//Remove from zombie list_t
				p_node = process_table[proc_id].zombie_list;

				if(p_node != NULL) {
					do {
						if((u32)(p_node->p_item) == thread_id) {
							//Free memory
							rtl_list_remove(&(process_table[proc_id].zombie_list),
							                p_node,
							                process_heap);
							mm_virt_free(thread_table[thread_id].kernel_stack,
							             KERNEL_STACK_SIZE,
							             MEM_UNCOMMIT | MEM_RELEASE);

							if(thread_table[thread_id].user_stack != NULL) {
								mm_virt_free(thread_table[thread_id].user_stack,
								             thread_table[thread_id].user_stack_size,
								             MEM_UNCOMMIT | MEM_RELEASE | MEM_USER);
							}

							exit_code = thread_table[thread_id].exit_code;
							pm_rls_spn_lock(&thread_table_lock);
							pm_rls_spn_lock(&process_table_lock);
							pm_set_errno(ESUCCESS);

							return exit_code;
						}

						p_node = p_node->p_next;
					} while(p_node != process_table[proc_id].zombie_list);

				}

				pm_rls_spn_lock(&thread_table_lock);
			}
		}

		pm_rls_spn_lock(&process_table_lock);

		if(pm_should_break()) {
			pm_set_errno(EINTR);
			return 0;
		}

		join_thread();
	}

	return 0;
}

bool pm_get_proc_id(u32 thread_id, u32* p_proc_id)
{
	//Check arguments
	if(thread_id >= MAX_THREAD_NUM) {
		pm_set_errno(EOVERFLOW);
		return false;
	}

	pm_acqr_spn_lock(&thread_table_lock);

	if(thread_table[thread_id].alloc_flag == false) {
		pm_rls_spn_lock(&thread_table_lock);
		pm_set_errno(ESRCH);
		return false;
	}

	*p_proc_id = thread_table[thread_id].process_id;
	pm_rls_spn_lock(&thread_table_lock);
	pm_set_errno(ESUCCESS);
	return true;
}

void pm_set_errno(u32 errno)
{
	thread_table[current_thread].errno = errno;
	return;
}

u32 pm_get_errno()
{
	return thread_table[current_thread].errno;
}

void pm_set_break(u32 thread_id, bool if_break)
{
	pm_acqr_spn_lock(&thread_table_lock);

	if(thread_table[thread_id].alloc_flag == false) {
		pm_rls_spn_lock(&thread_table_lock);
		pm_set_errno(ESRCH);
		return;
	}

	thread_table[thread_id].break_flag = if_break;

	pm_rls_spn_lock(&thread_table_lock);
	pm_set_errno(ESUCCESS);
	return;
}


bool pm_is_break(u32 thread_id)
{
	bool ret;
	pm_acqr_spn_lock(&thread_table_lock);

	if(thread_table[thread_id].alloc_flag == false) {
		pm_rls_spn_lock(&thread_table_lock);
		pm_set_errno(ESRCH);
		return false;
	}

	ret = thread_table[thread_id].break_flag;

	pm_rls_spn_lock(&thread_table_lock);
	pm_set_errno(ESUCCESS);
	return ret;
}

bool pm_should_break()
{
	return thread_table[current_thread].break_flag;
}

u32 pm_int_get_thread_pdt(u32 thread_id)
{
	return get_process_pdt(thread_table[thread_id].process_id);
}

void pm_enable_task_switch()
{
	cpu0_task_switch_enabled = true;
	return;
}

void pm_disable_task_switch()
{
	cpu0_task_switch_enabled = false;
	return;
}

u32 pm_get_thread_priority(u32 thread_id)
{
	u32 ret;
	pm_acqr_spn_lock(&thread_table_lock);

	if(thread_table[thread_id].alloc_flag == false) {
		pm_rls_spn_lock(&thread_table_lock);
		pm_set_errno(ESRCH);
		return 0;
	}

	ret = thread_table[thread_id].level;

	pm_rls_spn_lock(&thread_table_lock);
	pm_set_errno(ESUCCESS);
	return ret;

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

	proc_id = thread_table[thread_id].process_id;

	//Switch page table
	if(proc_id != current_process) {
		switch_process(proc_id);
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

	//Save&load fpu status
	__asm__ __volatile__(
	    "fnsave	(%0)\n\t"
	    "frstor	(%1)\n\t"
	    ::"r"(&(thread_table[current_thread].fpu_data)), "r"(&(thread_table[thread_id].fpu_data)));

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
	plist_node_t p_node;
	plist_node_t p_new_node;
	pthread_info_t p_info;
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

				if(task_queues[i].queue_t != NULL) {

					//Call the highest-int-level thread
					p_node = task_queues[i].queue_t;

					do {
						p_info = (pthread_info_t)(p_node->p_item);
						current_tick = io_get_tick_count();

						if(p_info->status == TASK_READY
						   || p_info->status == TASK_RUNNING) {
							p_info->status = TASK_RUNNING;
							pm_rls_raw_spn_lock(&(task_queues[i].lock));
							ret_id = ((u32)p_info - (u32)thread_table) / sizeof(thread_info_t);
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
					} while(p_node != task_queues[i].queue_t);
				}

				pm_rls_raw_spn_lock(&(task_queues[i].lock));

			} else if(i > INT_LEVEL_IDLE) {
				//Normal thread
				//Round Robin
				pm_acqr_raw_spn_lock(&(task_queues[i].lock));

				if(task_queues[i].queue_t != NULL) {

					//Look for TASK_READY thread
					p_node = task_queues[i].queue_t;

					do {
						p_info = (pthread_info_t)(p_node->p_item);
						current_tick = io_get_tick_count();

						if(p_info->status == TASK_READY
						   || p_info->status == TASK_RUNNING) {
							idle_flag = false;

							if(p_info->status_info.ready.time_slice > 0) {
								p_info->status = TASK_RUNNING;

								//Decrease time slice
								(p_info->status_info.ready.time_slice)--;

								if(p_info->status_info.ready.time_slice == 0) {
									rtl_list_remove(&(task_queues[i].queue_t),
									                p_node,
									                schedule_heap);

									p_new_node = rtl_list_insert_after(
									                 &(task_queues[i].queue_t),
									                 NULL,
									                 p_info,
									                 schedule_heap);

									if(p_new_node == NULL) {
										excpt_panic(EFAULT,
										            "Failes to append new item to task queue_t!\n");
									}

									p_info->p_task_queue_node = p_new_node;
								}

								pm_rls_raw_spn_lock(&(task_queues[i].lock));
								ret_id = ((u32)p_info - (u32)thread_table) / sizeof(thread_info_t);
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
					} while(p_node != task_queues[i].queue_t);
				}

				pm_rls_raw_spn_lock(&(task_queues[i].lock));

			} else if(idle_flag) {
				//IDLE thread
				pm_acqr_raw_spn_lock(&(task_queues[INT_LEVEL_IDLE].lock));

				if(task_queues[INT_LEVEL_IDLE].queue_t != NULL) {

					//Look for TASK_READY thread
					p_node = task_queues[INT_LEVEL_IDLE].queue_t;

					do {
						p_info = (pthread_info_t)(p_node->p_item);
						current_tick = io_get_tick_count();

						if(p_info->status == TASK_READY
						   || p_info->status == TASK_RUNNING) {
							idle_flag = false;

							if(p_info->status_info.ready.time_slice < 0) {
								p_info->status = TASK_RUNNING;

								//Decrease time slice
								(p_info->status_info.ready.time_slice)--;

								if(p_info->status_info.ready.time_slice == 0) {
									rtl_list_remove(&(task_queues[i].queue_t),
									                p_node,
									                schedule_heap);

									p_new_node = rtl_list_insert_after(
									                 &(task_queues[i].queue_t),
									                 NULL,
									                 p_info,
									                 schedule_heap);

									if(p_new_node == NULL) {
										excpt_panic(EFAULT,
										            "Failes to append new item to task queue_t!\n");
									}

									p_info->p_task_queue_node = p_new_node;
								}

								pm_rls_raw_spn_lock(&(task_queues[INT_LEVEL_IDLE].lock));
								ret_id = ((u32)p_info - (u32)thread_table) / sizeof(thread_info_t);
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
					} while(p_node != task_queues[i].queue_t);
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
	plist_node_t p_node;
	pthread_info_t p_info;

	//IDLE threads
	pm_acqr_raw_spn_lock(&(task_queues[INT_LEVEL_IDLE].lock));
	p_node = task_queues[INT_LEVEL_IDLE].queue_t;

	if(p_node != NULL) {
		p_info = (pthread_info_t)(p_node->p_item);

		if(p_info->status_info.ready.time_slice == 0) {
			//Reset time slice
			do {
				p_info = (pthread_info_t)(p_node->p_item);
				p_info->status_info.ready.time_slice = 1;
				p_node = p_node->p_next;
			} while(p_node != task_queues[INT_LEVEL_IDLE].queue_t);
		}
	}

	pm_rls_raw_spn_lock(&(task_queues[INT_LEVEL_IDLE].lock));

	//Normal thread
	for(i = 1; i <= INT_LEVEL_USR_HIGHEST; i++) {
		pm_acqr_raw_spn_lock(&(task_queues[i].lock));
		p_node = task_queues[i].queue_t;

		if(p_node != NULL) {
			p_info = (pthread_info_t)(p_node->p_item);

			//Reset time slice
			do {
				p_info = (pthread_info_t)(p_node->p_item);
				p_info->status_info.ready.time_slice = TIME_SLICE(i);
				p_node = p_node->p_next;
			} while(p_node != task_queues[i].queue_t);
		}

		pm_rls_spn_lock(&(task_queues[i].lock));
	}

	return;
}

void wait_thread()
{
	u32 level;
	u32 current_level;

	current_level = io_get_crrnt_int_level();
	pm_acqr_spn_lock(&thread_table_lock);

	//Check the status of the thread
	if(thread_table[current_thread].alloc_flag == false
	   || (thread_table[current_thread].status != TASK_READY
	       && thread_table[current_thread].status != TASK_RUNNING
	       && thread_table[current_thread].status != TASK_SLEEP)) {
		pm_rls_spn_lock(&thread_table_lock);
		pm_set_errno(ESRCH);
		return;
	}

	//Change status
	thread_table[current_thread].level = current_level;

	//Remove the thread from task queue_t
	level = thread_table[current_thread].level;

	pm_disable_task_switch();
	thread_table[current_thread].status = TASK_WAIT;
	pm_acqr_spn_lock(&(task_queues[level].lock));
	rtl_list_remove(
	    &(task_queues[level].queue_t),
	    thread_table[current_thread].p_task_queue_node,
	    schedule_heap);
	pm_rls_spn_lock(&(task_queues[level].lock));

	pm_enable_task_switch();

	pm_rls_spn_lock(&thread_table_lock);

	pm_schedule();

	pm_set_errno(ESUCCESS);

	return;
}

void release_thread(u32 id)
{
	pm_acqr_spn_lock(&thread_table_lock);

	if(thread_table[id].alloc_flag
	   && thread_table[id].status == TASK_ZOMBIE) {
		//Kernel stack_t
		if(thread_table[id].kernel_stack != NULL) {
			mm_virt_free(thread_table[id].kernel_stack,
			             KERNEL_STACK_SIZE,
			             MEM_UNCOMMIT | MEM_RELEASE);
		}

		thread_table[id].alloc_flag = false;

	} else {
		excpt_panic(EFAULT,
		            "Process table or thread table broken!\n");
	}

	pm_rls_spn_lock(&thread_table_lock);
	return;
}

void join_thread()
{
	u32 level;
	u32 current_level;

	current_level = io_get_crrnt_int_level();
	pm_acqr_spn_lock(&thread_table_lock);

	//Check the status of the thread
	if(thread_table[current_thread].alloc_flag == false
	   || (thread_table[current_thread].status != TASK_READY
	       && thread_table[current_thread].status != TASK_RUNNING
	       && thread_table[current_thread].status != TASK_SLEEP)) {
		pm_rls_spn_lock(&thread_table_lock);
		pm_set_errno(ESRCH);
		return;
	}

	//Change status
	thread_table[current_thread].level = current_level;

	//Remove the thread from task queue_t
	level = thread_table[current_thread].level;

	pm_disable_task_switch();
	thread_table[current_thread].status = TASK_JOIN;
	pm_acqr_spn_lock(&(task_queues[level].lock));
	rtl_list_remove(
	    &(task_queues[level].queue_t),
	    thread_table[current_thread].p_task_queue_node,
	    schedule_heap);
	pm_rls_spn_lock(&(task_queues[level].lock));
	pm_enable_task_switch();

	pm_rls_spn_lock(&thread_table_lock);

	pm_schedule();

	pm_set_errno(ESUCCESS);

	return;
}

void adjust_child_thread_stack(u32 ebp, u32 offset)
{
	u32* p_ebp;
	u32* p_esp;

	p_esp = (u32*)ebp;
	(*p_esp) += offset;

	p_ebp = (u32*)(*p_esp);
	(*p_ebp) += offset;

	return;
}

void user_thread_caller(u32 thread_id, void* p_args)
{
	//TODO:Allocate user stack_t and move to user space
	UNREFERRED_PARAMETER(thread_id);
	UNREFERRED_PARAMETER(p_args);
	return;
}
