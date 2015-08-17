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

#ifndef	SCHEDULE_H_INCLUDE
#define	SCHEDULE_H_INCLUDE

#include "../../../../common/common.h"
#include "../../../rtl/rtl.h"
#include "../../spinlock/arch/x86/spinlock.h"

#define	TIME_SLICE_TICKS		50
#define	TASK_QUEUE_HEAP_SIZE	4096
#define	MAX_TIME_SLICE_NUM		5

#pragma	pack(1)
typedef	struct _tss {
	u16		prev_task_link;
	u16		reserved0;
	u32		esp0;
	u16		ss0;
	u16		reserved1;
	u32		esp1;
	u16		ss1;
	u16		reserved2;
	u32		esp2;
	u16		ss2;
	u16		reserved3;
	u32		cr3;
	u32		eip;
	u32		eflags;
	u32		eax;
	u32		ecx;
	u32		edx;
	u32		ebx;
	u32		esp;
	u32		ebp;
	u32		esi;
	u32		edi;
	u16		es;
	u16		reserved4;
	u16		cs;
	u16		reserved5;
	u16		ss;
	u16		reserved6;
	u16		ds;
	u16		received7;
	u16		fs;
	u16		received8;
	u16		gs;
	u16		received9;
	u16		ldt_selector;
	u16		reserved10;
	u16		trap: 1;
	u16		reserved: 7;
	u16		io_map_base_addr;
} tss_t, *ptss_t;

#pragma	pack()

typedef	struct _context {
	u32		eax;
	u32		ebx;
	u32		ecx;
	u32		edx;
	u32		esi;
	u32		edi;
	u32		esp;
	u32		ebp;
	u32		eflags;
	u32		eip;
	u16		es;
	u16		cs;
	u16		ss;
	u16		ds;
	u16		fs;
	u16		gs;
} context_t, *pcontext_t;

typedef	struct {
	u32		start_tick;
	u32		stop_tick;
} sleep_thread_info_t, *psleep_thread_info_t;

typedef	struct {
	s32		time_slice;
} ready_thread_info_t, *pready_thread_info_t;

typedef	union {
	ready_thread_info_t	ready;
	sleep_thread_info_t 	sleep;
} thread_status_info_t, pthread_status_info_t;

typedef	struct {
	bool				alloc_flag;
	u32					process_id;
	u8					level;			//Interrupt level <=> thread priority
	u32					exit_code;
	plist_node_t			p_task_queue_node;
	bool				break_flag;
	u32					status;
	thread_status_info_t	status_info;
	void*				kernel_stack;
	void*				user_stack;
	size_t				user_stack_size;
	u32					errno;
	u32					ebp;			//Ring0
	u32					esp;			//Ring0
} thread_info_t, *pthread_info_t;

typedef	struct	{
	list_t		queue_t;
	spinlock_t	lock;
} task_queue_t, ptask_queue_t;

typedef	struct {
	thread_func		func;
	size_t			usr_stack_size;
	void*			p_args;
} usr_thread_info_t, *pusr_thread_info_t;


extern	u32				current_process;
extern	thread_info_t		thread_table[MAX_THREAD_NUM];

void	init_schedule();
s32		fork_thread(u32 new_proc_id);
void	adjust_int_level();
void	wait_thread();
void	release_thread(u32 id);

#endif	//!	SCHEDULE_H_INCLUDE

