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

#include "ramdisk.h"
#include "../../vfs.h"
#include "../../../debug/debug.h"
#include "../../../pm/pm.h"

static	void	kdriver_main(u32 thread_id, void* p_null);

void ramdisk_init()
{
	dbg_print("Initializing ramdisk...\n");

	//Create driver process
	if(pm_fork() == 0) {
		pm_exec("initrd", NULL);
		pm_clear_kernel_stack(kdriver_main, NULL);

	} else {
		pm_suspend_thrd(pm_get_crrnt_thrd_id());
	}

	return;
}


void kdriver_main(u32 thread_id, void* p_null)
{
	//Create driver
	//	pdriver_obj_t p_driver;

	//	p_driver = vfs_create_drv_object("initrd");
	//Create device
	//Awake thread 0
	pm_resume_thrd(0);

	//Message loop

	pm_exit_thrd(0);
	UNREFERRED_PARAMETER(thread_id);
	UNREFERRED_PARAMETER(p_null);
	return;
}
