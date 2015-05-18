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

#include "../../pm.h"

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
} tss, *ptss;

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
} context, *pcontext;

typedef	struct _thread_info {
	u32			process_id;
	u8			level;
	u32			ebp0;
	u32			esp0;
	u32			ebp3;
	u32			esp3;
} thread_info, *pthread_info;


void		pm_schedule();
u32			pm_create_thrd();
void		pm_terminate_thrd(u32 thread_id);
void		pm_suspend_thrd(u32 thread_id);
void		pm_resume_thrd(u32 thread_id);
void		pm_sleep(u32 ms);
u32			pm_get_crrnt_thrd_id();

#endif	//!	SCHEDULE_H_INCLUDE

