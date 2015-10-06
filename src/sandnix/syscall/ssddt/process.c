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

int ssddt_fork()
{
	return pm_fork();
}

void ssddt_execve(va_list p_args)
{
	//Agruments
	char *filename;
	char **argv;
	char **envp;

	//Variables
	char** p;
	u32 page;

	//Get args
	filename = va_arg(p_args, char*);
	argv = va_arg(p_args, char**);
	envp = va_arg(p_args, char**);

	//Check arguments
	if(!check_str_arg(filename, NAME_MAX + 1)) {
		pm_set_errno(EINVAL);
		return;
	}

	page = (u32)argv / PAGE_SIZE;

	if(!mm_virt_test(argv, 1, PG_STAT_COMMIT, true)) {
		pm_set_errno(EINVAL);
		return;
	}

	for(p = argv; *p != NULL; p++) {
		if((u32)p / PAGE_SIZE > page) {
			page = (u32)p / PAGE_SIZE;

			if(!mm_virt_test(p, 1, PG_STAT_COMMIT, true)) {
				pm_set_errno(EINVAL);
				return;
			}
		}

		if(!check_str_arg(*p, 4096)) {
			pm_set_errno(EINVAL);
			return;
		}
	}

	page = (u32)envp / PAGE_SIZE;

	if(!mm_virt_test(envp, 1, PG_STAT_COMMIT, true)) {
		pm_set_errno(EINVAL);
		return;
	}

	for(p = envp; *p != NULL; p++) {
		if((u32)p / PAGE_SIZE > page) {
			page = (u32)p / PAGE_SIZE;

			if(!mm_virt_test(p, 1, PG_STAT_COMMIT, true)) {
				pm_set_errno(EINVAL);
				return;
			}
		}

		if(!check_str_arg(*p, 4096)) {
			pm_set_errno(EINVAL);
			return;
		}
	}

	pm_execve(filename, argv, envp);

	return;
}

u32 ssddt_waitpid(va_list p_args)
{
	//Agruments
	u32 pid;
	u32 *status;
	u32 options;

	//Variables
	u32 ret;

	//Get args
	pid = va_arg(p_args, u32);
	status = va_arg(p_args, u32*);
	options = va_arg(p_args, u32);

	//check arguments
	if(!mm_virt_test(status, sizeof(u32), PG_STAT_COMMIT, true)) {
		pm_set_errno(EINVAL);
		return 0;
	}

	*status = pm_wait(pid, &ret, true);

	UNREFERRED_PARAMETER(options);
	return ret;
}

u32 ssddt_get_proc_id()
{
	return pm_get_crrnt_process();
}

u32 ssddt_get_uid()
{
	u32 euid;

	pm_get_proc_euid(pm_get_crrnt_process(), &euid);
	return euid;
}

k_status ssddt_set_uid(va_list p_args)
{
	//Agruments
	u32 uid;
	//Variables

	//Get args
	uid = va_arg(p_args, u32);

	if(pm_set_proc_euid(pm_get_crrnt_process(), uid)) {
		pm_set_errno(ESUCCESS);
		return ESUCCESS;

	} else {
		pm_set_errno(EINVAL);
		return EINVAL;
	}
}

u32 ssddt_get_gid()
{
	u32 gid;

	pm_get_proc_egid(pm_get_crrnt_process(), &gid);

	return gid;
}

k_status ssddt_set_gid(va_list p_args)
{
	//Agruments
	u32 gid;

	//Variables

	//Get args
	gid = va_arg(p_args, u32);

	if(pm_set_proc_egid(pm_get_crrnt_process(), gid)) {
		pm_set_errno(ESUCCESS);
		return ESUCCESS;

	} else {
		pm_set_errno(EINVAL);
		return EINVAL;
	}
}

void ssddt_chg_to_usr()
{
	pm_change_to_usr_process();
	return;
}
