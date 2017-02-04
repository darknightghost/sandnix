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

void		core_pm_thread_init();
void		core_pm_thread_core_init();
void		core_pm_thread_core_release();

u32			core_pm_thread_create(thread_func_t thread_func, void* p_arg);
void		core_pm_exit(u32 exit_code);
u32			core_pm_join(bool wait_threadid, u32 thread_id);
void		core_pm_suspend(u32 thread_id);
void		core_pm_resume(u32 thread_id);
u32			core_pm_get_crrnt_thread_id();
u32			core_pm_get_thrd_priority(u32 thrd_id);
void		core_pm_set_thrd_priority(u32 thrd_id, u32 priority);
void        core_pm_schedule();
void		core_pm_idle();
