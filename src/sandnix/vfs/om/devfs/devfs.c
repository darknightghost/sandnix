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
#include "../../../rtl/rtl.h"

static	void			kdriver_main(u32 thread_id, void* p_null);
static	bool			dispatch_message(pmsg_t p_msg);
static	void			on_open(pmsg_t p_msg);
static	void			on_access(pmsg_t p_msg);
static	void			on_readdir(pmsg_t p_msg);
static	bool			check_privilege(u32 euid, u32 egid, pdevice_obj_t p_dev);
static	pdevice_obj_t	get_dev_by_path(char* path);
static	u32				name_to_dev_num(char* name);
static	u32				get_dir_file_obj(char* path);

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
	devfs_driver = p_driver->driver_id;

	//Create device
	p_device = vfs_create_dev_object("devfs");
	p_device->gid = 0;
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
		msg_complete(p_msg ENOTSUP);
	}

	return true;
}

void on_open(pmsg_t p_msg)
{
	pmsg_open_info_t p_info;
	pdevice_obj_t p_dev;
	u32 euid;
	u32 egid;
	k_status status;

	//Check buf type
	if(!p_msg->flags.properties.direct_buf) {
		msg_complete(p_msg, EINVAL);
		return;
	}

	p_info = p_msg->buf.addr;

	if(p_info->flags | O_DIRECTORY) {
		p_info->file_object = get_dir_file_obj(&(p_info->path));
		p_info->size = 0;

		status = pm_get_errno();
		msg_complete(p_msg, status);

		return;


	} else {
		//Get device object
		p_dev = get_dev_by_path(&(p_info->path));

		if(p_dev == NULL) {
			msg_complete(p_msg, ENFILE);
			return;
		}

		//Check privilege
		euid = pm_get_proc_euid(p_info->process);
		egid = pm_get_proc_egid(p_info->process);

		if(p_info->mode | O_CREAT) {
			msg_complete(p_msg, EACCES);
			return;
		}

		if(!check_privilege(euid, egid, p_dev)) {
			msg_complete(p_msg, EACCES);
			return;
		}

		p_info->file_object = (pfile_obj_t)p_dev;
		p_info->file_size = 0;

		msg_complete(p_msg, ESUCCESS);
		return;
	}
}

void on_access(pmsg_t p_msg)
{
}

void on_readdir(pmsg_t p_msg)
{

}

bool check_privilege(u32 euid, u32 egid, pdevice_obj_t p_dev)
{
	if(euid == 0) {
		return true;
	}

	if(egid == p_dev->gid) {
		return true;
	}

	return false;
}

pdevice_obj_t get_dev_by_path(char* path)
{
	char name_buf[NAME_MAX];
	char* p_next;
	pdevice_obj_t p_dev;
	pdev_mj_info_t p_info;
	u32 dev_num;

	//Analyse path
	p_next = path;

	if(rtl_get_next_name_in_path(&p_next, name_buf, NAME_MAX) != ESUCCESS) {
		return NULL;
	}

	//Check the name is a device or a major number
	p_dev = get_dev_by_name(name_buf);

	if(p_dev == NULL) {
		//The name is a major number
		p_info = get_mj_by_name(name_buf);

		if(p_info == NULL) {
			return NULL;
		}

		//Get device
		if(rtl_get_next_name_in_path(&p_next,
		                             name_buf, NAME_MAX) != ESUCCESS) {
			return NULL;
		}

		if(*p_next != '\0') {
			return NULL;
		}

		dev_num = name_to_dev_num(name_buf);

		if(!OPERATE_SUCCESS) {
			return NULL;
		}

		p_dev = get_dev(dev_num);

	}

	return p_dev;
}

u32 name_to_dev_num(char* name)
{
	char* p;
	u32 mj_num;
	u32 mn_num;
	u32 dev_num;

	//Get major number
	mj_num = 0;

	for(p = name; *p != ':'; p++) {
		if(*p == '\0') {
			pm_set_errno(EINVAL);
			return 0;
		}

		if(*p > '9' || *p < '0') {
			pm_set_errno(EINVAL);
			return 0;
		}

		mj_num = mj_num * 10 + *p - '0';
	}

	p++;

	if(*p == '\0') {
		pm_set_errno(EINVAL);
		return 0;
	}

	//Get minor number
	for(; *p != '\0'; p++) {
		if(*p > '9' || *p < '0') {
			pm_set_errno(EINVAL);
			return 0;
		}

		mn_num = mn_num * 10 + *p - '0';
	}

	dev_num = MK_DEV(mj_num.mn_num);

	pm_set_errno(ESUCCESS);
	return dev_num;
}

u32 get_dir_file_obj(char* path)
{
	char* p;
	char name_buf[NAME_MAX];
	pdev_mj_info_t p_info;

	p = path;

	if(rtl_get_next_name_in_path(&p, name_buf, NAME_MAX) != ESUCCESS) {
		pm_set_errno(ENFILE);
		return INVALID_FILEID;
	}

	while(*p == '/') {
		p++;
	}

	if(*p != NULL) {
		pm_set_errno(ENFILE);
		return INVALID_FILEID;
	}

	p_info = get_mj_by_name(name_buf);

	if(p_info == NULL) {
		pm_set_errno(ENFILE);
		return INVALID_FILEID;
	}

	pm_set_errno(ESUCCESS);
	return p_info->file_obj.file_id;
}
