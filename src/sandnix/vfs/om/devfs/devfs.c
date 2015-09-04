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

#include "devfs.h"

static	void	kdriver_main(u32 thread_id, void* p_null);
static	bool	dispatch_message(pmsg_t p_msg);
static	void	on_open(pmsg_t p_msg);
static	void	on_access(pmsg_t p_msg);
static	void	on_readdir(pmsg_t p_msg);

void devfs_init()
{
	dbg_print("Initializing devfs...\n");

	//Create driver process
	if(pm_fork() == 0) {
		pm_exec("devfs", NULL);
		pm_clear_kernel_stack(kdriver_main, NULL);

	} else {
		pm_suspend_thrd(pm_get_crrnt_thrd_id());
	}

	return;
}

void kdriver_main(u32 thread_id, void* p_null)
{
	pdevice_obj_t p_device;
	pmsg_t p_msg;

	//Create driver
	pdriver_obj_t p_driver;

	p_driver = vfs_create_drv_object("devfs");
	p_driver->process_id = pm_get_crrnt_process();
	vfs_reg_driver(p_driver);

	//Create device
	p_device = vfs_create_dev_object("devfs");
	p_device->device_number = MK_DEV(vfs_get_dev_major_by_name("volume",
	                                 DEV_TYPE_BLOCK),
	                                 0);
	vfs_add_device(p_device, p_driver->driver_id);

	//Awake thread 0
	pm_resume_thrd(0);

	//Message loop
	do {
		vfs_recv_drv_message(p_driver->driver_id, &p_msg, true);
	} while(dispatch_message(p_msg));

	pm_exit_thrd(0);
	UNREFERRED_PARAMETER(thread_id);
	UNREFERRED_PARAMETER(p_null);
	return;
}

bool dispatch_message(pmsg_t p_msg)
{
	switch(p_msg->message) {
	case MSG_OPEN:
		on_open(p_msg);
		break;

	case MSG_ACCESS:
		on_access(p_msg);
		break;

	case MSG_READDIR:
		on_readdir(p_msg);
		break;

	default:
		msg_complete(p_msp, ENOTSUP);
	}

	return true;
}
