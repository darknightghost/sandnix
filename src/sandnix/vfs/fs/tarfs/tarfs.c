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

#include "tarfs.h"
#include "../../vfs.h"
#include "../../../debug/debug.h"
#include "../../../rtl/rtl.h"
#include "../../../exceptions/exceptions.h"
#include "tar.h"
#include "fs_structs.h"
#include "../ramdisk/ramdisk.h"

u32		initrd_volume;
u32		initrd_fs;

static	u32				tarfs_driver;
static	u32				fs_file_id;
static	array_list_t	volumes;
static	mutex_t			volume_list_lock;
static	void*			fs_heap;
static	u32				mount_thread_id;

static	void	kdriver_main(u32 thread_id, void* p_null);
static	bool	dispatch_message(pmsg_t p_msg);
static	void	mount_thread_func(u32 thread_id,void* p_null);
static	void	volume_thread_func(u32 thread_id,void* volume);
static	void	on_mount(pmsg_t p_msg);
static	void	on_umount(pmsg_t p_msg);
static	void	on_open(pmsg_t p_msg);
static	void	on_read(pmsg_t p_msg);
static	void	on_write(pmsg_t p_msg);
static	void	on_readdir(pmsg_t p_msg);
static	void	on_access(pmsg_t p_msg);
static	void	on_fstat(pmsg_t p_msg);
static	void	on_close(pmsg_t p_msg);

static	k_status	analyse_inodes();
static	u32			create_volume(u32 dev_num);

void tarfs_init()
{
	k_status status;
	
	dbg_print("Initializing tarfs...\n");
	
	//Create heap
	fs_heap = mm_hp_create(4096,HEAP_EXTENDABLE | HEAP_MULTITHREAD);
	if(fs_heap == NULL){
		excpt_panic(EFAULT,
			"Failed to create tarfs heap!\n");
	}

	//Initialize data structures
	status = rtl_array_list_init(&volumes,64,fs_heap);
	if(status != ESUCCESS){
		excpt_panic(status,
			"Failed to initialize volume list of tarfs!\n");
	}
	pm_init_mutex(&volume_list_lock);
	
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
	pdriver_obj_t p_driver;
	k_status status;
	pdriver_obj_t p_driver;

	//Create driver
	p_driver = vfs_create_drv_object("tarfs");
	p_driver->process_id = pm_get_crrnt_process();
	vfs_reg_driver(p_driver);
	tarfs_driver = p_driver->driver_id;

	//Create fs device
	p_device = vfs_create_dev_object("tarfs");
	p_device->gid = 0;
	p_device->device_number = MK_DEV(vfs_get_dev_major_by_name("filesystem",
	                                 DEV_TYPE_CHAR),
	                                 0);
	p_device->block_size = 1;
	vfs_add_device(p_device, p_driver->driver_id);
	initrd_fs = p_device->device_number;
	fs_file_id = p_device->file_obj.file_id;
	
	//Create mount thread
	mount_thread_id = pm_create_thrd(mount_thread_func,
		true,
		false,
		PRIORITY_DRVNORMAL,
		NULL);
	status = pm_get_errno();
	if(status != ESUCCESS){
		excpt_panic(status,
			"Unable to create mount thread for tarfs!\n");
	}

	//Create volume device
	initrd_volume = create_volume(initrd_ramdisk);
	
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
	if(p_msg->file_id == fs_file_id){
		//Mount thread
	}else{
		//Volume thread
	}
	
	return true;
}