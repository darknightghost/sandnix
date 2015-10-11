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

#include "ssddt.h"
#include "ssddt_funcs.h"
#include "../../rtl/rtl.h"
#include "../../vfs/vfs.h"
#include "../../msg/msg.h"
#include "../../mm/mm.h"
#include "../../pm/pm.h"
#include "../../exceptions/exceptions.h"

void ssddt_schedule()
{
	pm_schedule();
	return;
}

u32 ssddt_create_thrd(va_list p_args)
{
	//Agruments
	thread_func start_addr;
	u32 priority;
	bool is_ready;
	void* args;

	//Variables

	//Get args
	start_addr = va_arg(p_args, thread_func);
	priority = va_arg(p_args, u32);
	is_ready = va_arg(p_args, bool);
	args = va_arg(p_args, void*);

	//Check arguments
	if(!mm_virt_test(start_addr, 1, PG_STAT_COMMIT, true)) {
		pm_set_errno(EINVAL);
		return EINVAL;
	}

	return pm_create_thrd(start_addr, is_ready, true, priority, args);
}

void ssddt_exit_thrd(va_list p_args)
{
	//Agruments
	u32 exit_code;

	//Variables

	//Get args
	exit_code = va_arg(p_args, u32);
	pm_exit_thrd(exit_code);

	return;
}

void ssddt_suspend(va_list p_args)
{
	//Agruments
	u32 thread_id;

	//Variables
	u32 process;

	//Get args
	thread_id = va_arg(p_args, u32);

	if(!pm_get_proc_id(thread_id, &process)) {
		pm_set_errno(ESRCH);
		return;
	}

	if(process != pm_get_crrnt_process()) {
		pm_set_errno(EACCES);
		return;
	}

	pm_suspend_thrd(thread_id);

	return;
}

u32 ssddt_join(va_list p_args)
{
	//Agruments
	u32 thread_id;

	//Variables

	//Get args
	thread_id = va_arg(p_args, u32);

	return pm_join(thread_id);
}

void ssddt_resume(va_list p_args)
{
	//Agruments
	u32 thread_id;

	//Variables
	u32 process;

	//Get args
	thread_id = va_arg(p_args, u32);

	if(!pm_get_proc_id(thread_id, &process)) {
		pm_set_errno(ESRCH);
		return;
	}

	if(process != pm_get_crrnt_process()) {
		pm_set_errno(EACCES);
		return;
	}

	pm_resume_thrd(thread_id);

	return;
}

void ssddt_sleep(va_list p_args)
{
	//Agruments
	u32 ms;

	//Variables

	//Get args
	ms = va_arg(p_args, u32);

	pm_sleep(ms);

	return;
}

u32 ssddt_get_thrd_id()
{
	return pm_get_crrnt_thrd_id();
}

u32 ssddt_get_errno()
{
	return pm_get_errno();
}

void ssddt_set_errno(va_list p_args)
{
	//Agruments
	u32 errno;

	//Variables

	//Get args
	errno = va_arg(p_args, u32);

	pm_set_errno(errno);

	return;
}
