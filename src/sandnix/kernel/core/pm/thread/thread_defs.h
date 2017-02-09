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
#include "../../rtl/rtl.h"

#include "thread_ref_obj_defs.h"

#include "../lock/spinlock/spnlck.h"

#define	MAX_TIME_SLICE_NUM		8

#define PRIORITY_HIGHEST		0x000000FF
#define PRIORITY_LOWEST			0x00000000

#define PRIORITY_IDLE			0x00000000
#define PRIORITY_IDLE_TASK		0x00000001
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

//void* thread_func(u32 thread_id, void* p_arg);
typedef	void*	(*thread_func_t)(u32, void*);

#include "thread_obj_defs.h"
typedef	struct _thread_obj		thread_obj_t, *pthread_obj_t;
typedef	struct	_core_sched_info {
    bool			enabled;
    spnlck_t		lock;
    u32				priority;
    volatile u64	cpu_use_stat_h;
    volatile u64	cpu_use_stat_l;
    plist_node_t	current_node;
    pthread_obj_t	p_idle_thread;
} core_sched_info_t, *pcore_sched_info_t;
