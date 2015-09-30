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

#include "../../vfs.h"
#include "devfs.h"
#include "../../../rtl/rtl.h"
#include "../../../debug/debug.h"

static	u32				volume_dev;
static	u32				fs_dev;
static	u32				volume_file_id;
static	u32				fs_file_id;

static	void			kdriver_main(u32 thread_id, void* p_null);
static	bool			dispatch_message(pmsg_t p_msg);
static	void			on_open(pmsg_t p_msg);
static	void			on_access(pmsg_t p_msg);
static	void			on_stat(pmsg_t p_msg);
static	void			on_readdir(pmsg_t p_msg);
static	void			on_mount(pmsg_t p_msg);
static	void			on_umount(pmsg_t p_msg);
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
	pdriver_obj_t p_driver;

	//Create driver
	p_driver = vfs_create_drv_object("devfs");
	p_driver->process_id = pm_get_crrnt_process();
	vfs_reg_driver(p_driver);
	devfs_driver = p_driver->driver_id;

	//Create filesystem device
	p_device = vfs_create_dev_object("devfs");
	p_device->gid = 0;
	p_device->device_number = MK_DEV(vfs_get_dev_major_by_name("filesystem",
	                                 DEV_TYPE_CHAR),
	                                 0);
	p_device->block_size = 1;
	vfs_add_device(p_device, p_driver->driver_id);
	fs_dev = p_device->device_number;
	fs_file_id = p_device->file_obj.file_id;

	//Create volume device
	p_device = vfs_create_dev_object("udev");
	p_device->gid = 0;
	p_device->device_number = MK_DEV(vfs_get_dev_major_by_name("volume",
	                                 DEV_TYPE_CHAR),
	                                 0);
	p_device->block_size = 1;
	vfs_add_device(p_device, p_driver->driver_id);
	volume_dev = p_device->device_number;
	volume_file_id = p_device->file_obj.file_id;

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
	if(p_msg->file_id == fs_file_id) {
		switch(p_msg->message) {
		case MSG_MOUNT:
			on_mount(p_msg);
			break;

		case MSG_UMOUNT:
			on_umount(p_msg);
			break;

		default:
			msg_complete(p_msg, ENOTSUP);
		}

	} else if(p_msg->file_id == volume_file_id) {
		switch(p_msg->message) {
		case MSG_OPEN:
			on_open(p_msg);
			break;

		case MSG_ACCESS:
			on_access(p_msg);
			break;

		case MSG_STAT:
			on_stat(p_msg);
			break;

		case MSG_READDIR:
			on_readdir(p_msg);
			break;

		case MSG_SYNC:
			msg_complete(p_msg, ESUCCESS);
			break;

		default:
			msg_complete(p_msg, ENOTSUP);
		}

	} else {
		msg_complete(p_msg, ENXIO);
	}

	return true;
}

void on_mount(pmsg_t p_msg)
{
	pmsg_mount_info_t p_info;

	if(!(p_msg->flags.properties.pmo_buf)) {
		msg_complete(p_msg, EINVAL);
	}

	p_info = mm_pmo_map(NULL, p_msg->buf.pmo_addr, false);

	if(p_info == NULL) {
		msg_complete(p_msg, EFAULT);
	}

	p_info->volume_dev = volume_dev;
	p_info->mode = S_IRUSR | S_IRGRP | S_IROTH;
	mm_pmo_unmap(p_info, p_msg->buf.pmo_addr);
	msg_complete(p_msg, ESUCCESS);

	return;
}

void on_umount(pmsg_t p_msg)
{
	msg_complete(p_msg, ESUCCESS);
	return;
}

void on_open(pmsg_t p_msg)
{
	pmsg_open_info_t p_info;
	pdevice_obj_t p_dev;
	u32 euid;
	u32 egid;
	k_status status;

	//Check buf type
	if(!p_msg->flags.properties.pmo_buf) {
		msg_complete(p_msg, EINVAL);
		return;
	}

	//Map bufffer
	p_info = mm_pmo_map(NULL, p_msg->buf.pmo_addr, false);

	if(p_info == NULL) {
		msg_complete(p_msg, EFAULT);
		return;
	}

	p_info->serial_read = false;

	if(p_info->flags | O_DIRECTORY) {
		//Directory
		p_info->file_object = get_dir_file_obj(&(p_info->path));
		p_info->file_size = 0;

		status = pm_get_errno();
		msg_complete(p_msg, status);
		mm_pmo_unmap(p_info, p_msg->buf.pmo_addr);

		return;

	} else {
		//Get device object
		p_dev = get_dev_by_path(&(p_info->path));

		if(p_dev == NULL) {
			msg_complete(p_msg, ENFILE);
			mm_pmo_unmap(p_info, p_msg->buf.pmo_addr);
			return;
		}

		//Check privilege
		pm_get_proc_euid(p_info->process, &euid);
		pm_get_proc_egid(p_info->process, &egid);

		if(p_info->mode | O_CREAT) {
			msg_complete(p_msg, EACCES);
			mm_pmo_unmap(p_info, p_msg->buf.pmo_addr);
			return;
		}

		if(!check_privilege(euid, egid, p_dev)) {
			msg_complete(p_msg, EACCES);
			mm_pmo_unmap(p_info, p_msg->buf.pmo_addr);
			return;
		}

		p_info->file_object = p_dev->file_obj.file_id;
		p_info->file_size = 0;

		vfs_msg_forward(p_msg, p_dev->device_number);
		mm_pmo_unmap(p_info, p_msg->buf.pmo_addr);
		return;
	}
}

void on_access(pmsg_t p_msg)
{
	pmsg_access_info_t p_info;
	pdevice_obj_t p_dev;
	u32 euid;
	u32 egid;

	//Check buf type
	if(!p_msg->flags.properties.direct_buf) {
		msg_complete(p_msg, EINVAL);
		return;
	}

	p_info = p_msg->buf.addr;

	//Devices cannot be executed
	if(p_info->mode | X_OK) {
		msg_complete(p_msg, EACCES);
		return;
	}

	//Is a device?
	p_dev = get_dev_by_path(&(p_info->path));

	if(p_dev == NULL) {
		//Is a directory?
		if(p_info->mode | W_OK) {
			//Directories cannot be written
			msg_complete(p_msg, EACCES);
			return;
		}

		get_dir_file_obj(&(p_info->path));

		if(!OPERATE_SUCCESS) {
			msg_complete(p_msg, ENFILE);
			return;
		}
	}

	if(p_info->mode == F_OK) {
		msg_complete(p_msg, ESUCCESS);
		return;

	} else {
		//Check privilege
		pm_get_proc_euid(p_info->process, &euid);
		pm_get_proc_egid(p_info->process, &egid);

		if(p_dev->gid == egid || euid == 0) {
			msg_complete(p_msg, ESUCCESS);
			return;

		} else {
			msg_complete(p_msg, EACCES);
			return;
		}
	}
}

void on_stat(pmsg_t p_msg)
{
	pmsg_stat_info_t p_info;
	pmsg_stat_data_t p_data;
	pdevice_obj_t p_dev;
	pdev_mj_info_t p_mj;

	//Check buf type
	if(!p_msg->flags.properties.pmo_buf) {
		msg_complete(p_msg, EINVAL);
		return;
	}

	//Map bufffer
	p_info = mm_pmo_map(NULL, p_msg->buf.pmo_addr, false);
	p_data = (pmsg_stat_data_t)p_info;

	if(p_info == NULL) {
		msg_complete(p_msg, EFAULT);
		return;
	}

	//Get device object
	p_dev = get_dev_by_path(&(p_info->path));

	if(p_dev == NULL) {
		p_mj = (pdev_mj_info_t)get_file_obj(get_dir_file_obj(&(p_info->path)));

		if(p_mj == NULL) {
			msg_complete(p_msg, ENFILE);
			mm_pmo_unmap(p_info, p_msg->buf.pmo_addr);
			return;
		}

		//Directory
		p_data->stat.atime = 0;
		p_data->stat.block_num = 0;
		p_data->stat.block_size = 0;
		p_data->stat.ctime = 0;
		p_data->stat.dev_num = 0;
		p_data->stat.gid = 0;
		p_data->stat.inode = 0 - (p_mj->mj_num) - 1;
		p_data->stat.mode = S_IRUSR | S_IRGRP | S_IROTH;
		p_data->stat.mtime = 0;
		p_data->stat.nlink = 1;
		p_data->stat.rdev = 0;
		p_data->stat.size = 0;
		p_data->stat.uid = 0;

		msg_complete(p_msg, ESUCCESS);

	} else {
		//Device
		p_data->stat.atime = 0;
		p_data->stat.block_num = 0;
		p_data->stat.block_size = p_dev->block_size;
		p_data->stat.ctime = 0;
		p_data->stat.dev_num = p_dev->device_number;
		p_data->stat.gid = p_dev->gid;
		p_data->stat.inode = p_dev->device_number;
		p_data->stat.mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
		p_data->stat.mtime = 0;

		if(p_dev->file_obj.obj.name == NULL) {
			p_data->stat.nlink = 1;

		} else {
			p_data->stat.nlink = 2;
		}

		p_data->stat.rdev = p_dev->device_number;
		p_data->stat.size = 0;
		p_data->stat.uid = 0;

		msg_complete(p_msg, ESUCCESS);

	}

	mm_pmo_unmap(p_info, p_msg->buf.pmo_addr);
	return;
}

void on_readdir(pmsg_t p_msg)
{
	pmsg_readdir_info_t p_info;
	pmsg_readdir_data_t p_data;
	pdevice_obj_t p_dev;
	pfile_obj_t p_fo;
	size_t count;
	size_t read_entries;
	size_t offset;

	//Check buf type
	if(!p_msg->flags.properties.pmo_buf) {
		msg_complete(p_msg, EINVAL);
		return;
	}

	//Map bufffer
	p_info = mm_pmo_map(NULL, p_msg->buf.pmo_addr, false);

	if(p_info == NULL) {
		msg_complete(p_msg, EFAULT);
		return;
	}

	p_data = (pmsg_readdir_data_t)p_info;
	p_fo = get_file_obj(p_info->file_obj);
	count = p_info->count;
	offset = p_info->offset;

	if(OBJ_MINOR_CLASS((pkobject_t)p_fo) == OBJ_MN_DEVICE) {
		//Volume device
		p_dev = (pdevice_obj_t)p_fo;

		if(volume_dev != p_dev->device_number) {
			mm_pmo_unmap(p_info, p_msg->buf.pmo_addr);
			msg_complete(p_msg, ENOTDIR);
			return;
		}

		read_entries = get_devfs_root((pdirent_t)(&(p_data->data)),
		                              offset,
		                              count);
		p_data->count = read_entries;

		msg_complete(p_msg, ESUCCESS);
		return;

	} else {
		//Directory
		read_entries = get_devfs_dir((pdev_mj_info_t)p_fo,
		                             (pdirent_t)(&(p_data->data)),
		                             offset,
		                             count);
		p_data->count = read_entries;

		msg_complete(p_msg, ESUCCESS);
		return;

	}

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

	dev_num = MK_DEV(mj_num, mn_num);

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

	if(*p != '\0') {
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
