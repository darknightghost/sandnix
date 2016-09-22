/*
    Copyright 2016,王思远 <darknightghost.cn@gmail.com>

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

#pragma once

#include "../../../../../common/common.h"
#include "thread_ref_obj.h"

#define PRIORITY_HIGHEST		0x000000FF
#define PRIORITY_LOWEST			0x00000000

#define PRIORITY_IDLE			0x00000000
#define PRIORITY_USER_NORMAL	0x00000014
#define PRIORITY_USER_HIGHEST	0x00000028

#define	PRIORITY_KRNL_NORMAL	0x00000030
#define PRIORITY_DISPATCH		0x00000040
#define PRIORITY_IRQ			0x00000050
#define	PRIORITY_EXCEPTION		0x000000FF

#define PROCESS_ALIVE			0x00000000
#define PROCESS_ZOMBIE			0x00000001

#define TASK_RUNNING			0x00000000
#define TASK_READY				0x00000001
#define TASK_SUSPEND			0x00000002
#define TASK_SLEEP				0x00000003
#define TASK_ZOMBIE				0x00000004

//void thread_func(u32 thread_id, void* p_arg);
typedef	void	(*thread_func_t)(u32, void*);

u32			core_pm_thread_create(thread_func_t thread_func, void* p_arg);
void		core_pm_exit(u32 exit_code);
u32			core_pm_join(bool wait_threadid, u32 thread_id);
void		core_pm_suspend(u32 thread_id);
void		core_pm_resume(u32 thread_id);
u32			core_pm_get_crrnt_thread_id();
u32			core_pm_get_thrd_priority(u32 thrd_id);
void		core_pm_set_thrd_priority(u32 thrd_id, u32 priority);
void		core_pm_schedule();
