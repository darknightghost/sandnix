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

u32				initrd_ramdisk;

static	void	kdriver_main(u32 thread_id, void* p_null);
static	bool	dispatch_message(pmsg_t p_msg);
static	void	on_open(pmsg_t p_msg);
static	void	on_read(pmsg_t p_msg);
static	void	on_close(pmsg_t p_msg);
static	void	on_destroy(pmsg_t p_msg);
static	size_t	ramdisk_size;
static	char*	p_ramdisk;

void ramdisk_init()
{
	dbg_print("Initializing ramdisk...\n");

	//Get ramdisk info
	ramdisk_size = *((u32*)RAMDISK_BASE);
	p_ramdisk = (char*)(RAMDISK_BASE + 4);

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
	pdevice_obj_t p_device;
	pmsg_t p_msg;

	//Create driver
	pdriver_obj_t p_driver;

	p_driver = vfs_create_drv_object("initrd");
	p_driver->process_id = pm_get_crrnt_process();
	vfs_reg_driver(p_driver);

	//Create device
	p_device = vfs_create_dev_object("initrd");
	p_device->device_number = MK_DEV(vfs_get_dev_major_by_name("ramdisk",
	                                 DEV_TYPE_BLOCK),
	                                 0);
	vfs_add_device(p_device, p_driver->driver_id);
	initrd_ramdisk = p_device->device_number;

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

	case MSG_READ:
		on_read(p_msg);
		break;

	case MSG_CLOSE:
		on_close(p_msg);
		break;

	case MSG_DESTROY:
		on_destroy(p_msg);
		return false;

	default:
		msg_complete(p_msg, ENOTSUP);
	}

	return true;
}

void on_open(pmsg_t p_msg)
{
	pmsg_open_info_t p_info;

	p_info = mm_pmo_map(NULL, p_msg->buf.pmo_addr, false);

	if(p_info == NULL) {
		msg_complete(p_msg, EFAULT);
		return;
	}

	p_info->file_size = ramdisk_size;
	p_info->serial_read = false;

	mm_pmo_unmap(p_info, p_msg->buf.pmo_addr);
	msg_complete(p_msg, ESUCCESS);
	return;
}

void on_read(pmsg_t p_msg)
{
	pmsg_read_info_t p_info;
	pmsg_read_data_t p_data;
	size_t len;
	size_t offset;

	//Check buf type
	if(!p_msg->flags.properly.pmo_buf) {
		msg_complete(p_msg, EINVAL);
		return;
	}

	//Map buffer
	p_info = mm_pmo_map(NULL, p_msg->buf.pmo_addr, false);

	if(p_info == NULL) {
		msg_complete(p_msg, EFAULT);
		return;
	}

	len = p_info->len;
	offset = p_info->offset;
	p_data = (pmsg_read_data_t)p_info;

	//Compute length to read
	if(offset + len > ramdisk_size) {
		if(offset > ramdisk_size) {
			p_data->len = 0;
			mm_pmo_unmap(p_info, p_msg->buf.pmo_addr);
			msg_complete(p_msg, ESUCCESS);
			return;
		}

		len = ramdisk_size - offset;
	}

	//Read data
	p_data->len = len;
	rtl_memcpy(&(p_data->data), p_ramdisk + offset, len);

	mm_pmo_unmap(p_info, p_msg->buf.pmo_addr);
	msg_complete(p_msg, ESUCCESS);
	return;
}

void on_close(pmsg_t p_msg)
{
	msg_complete(p_msg, ESUCCESS);
	return;
}

void on_destroy(pmsg_t p_msg)
{
	msg_complete(p_msg, ESUCCESS);
	return;
}
