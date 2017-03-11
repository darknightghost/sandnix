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

#include "./process_defs.h"

#define	MODULE_NAME		core_pm

void		core_pm_reg_proc_ref_obj(proc_ref_call_back_t callback);
u32			core_pm_fork(void* child_start_address);
u32			core_pm_wait(bool wait_pid, u32 process_id);
u32			core_pm_get_subsys(u32 pid);
kstatus_t	core_pm_set_subsys(u32 pid, u32 subsys_id);
u32			core_pm_get_uid(u32 pid);
u32			core_pm_get_gid(u32 pid);
u32			core_pm_get_euid(u32 pid);
kstatus_t	core_pm_set_euid(u32 pid, u32 euid);
u32			core_pm_get_egid(u32 pid);
kstatus_t	core_pm_set_egid(u32 pid, u32 egid);
void		core_pm_set_groups(u32* groupids, size_t size);
size_t		core_pm_get_groups(u32* buf, size_t buf_size);

void		PRIVATE(process_init)();
void		PRIVATE(add_thread)(u32 process_id, u32 thread_id);
void		PRIVATE(zombie_process_thrd)(u32 process_id, u32 thread_id);
void		PRIVATE(remove_process_thrd)(u32 process_id, u32 thread_id);
void		PRIVATE(release_proc_id)(u32 id);

#undef	MODULE_NAME
