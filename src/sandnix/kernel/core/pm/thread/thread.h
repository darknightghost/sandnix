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

#include "./thread_defs.h"

#define	MODULE_NAME		core_pm

void		core_pm_reg_thread_ref_obj(thread_ref_call_back_t callback);
u32			core_pm_thread_create(thread_func_t thread_func, u32 k_stack_size,
                                  u32 priority, void* p_arg);
void		core_pm_exit(void* retval);
u32			core_pm_join(bool wait_threadid, u32 thread_id, void** p_retval);
void		core_pm_suspend(u32 thread_id);
void		core_pm_resume(u32 thread_id);
void		core_pm_sleep(u64* p_ns);
u32			core_pm_get_currnt_thread_id();
u32			core_pm_get_currnt_proc_id();
u32			core_pm_get_currnt_thrd_priority();
void		core_pm_set_currnt_thrd_priority(u32 priority);
u32			core_pm_get_thrd_priority(u32 thrd_id);
void		core_pm_set_thrd_priority(u32 thrd_id, u32 priority);
void        core_pm_schedule();
void		core_pm_disable_sched();
void		core_pm_enable_sched();
void		core_pm_idle();


void		PRIVATE(thread_init)();
void		PRIVATE(thread_core_init)();
void		PRIVATE(thread_core_release)();
void		PRIVATE(thread_id_release)(u32 id);

//Add reference to thread object
void		PRIVATE(ref_thread)(u32 id);

//Decrease reference to thread object
void		PRIVATE(unref_thread)(u32 id);

#undef	MODULE_NAME
