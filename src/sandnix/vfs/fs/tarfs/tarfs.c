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
#include "../../../pm/pm.h"
#include "../../../../common/tar.h"
#include "fs_structs.h"
#include "../ramdisk/ramdisk.h"

#define	MAX_INODE_NUM	1024
#define	MAX_DIRENT_NUM	512

u32		initrd_volume;

static	u32				tarfs_driver;
static	void*			fs_heap;
static	u32				initrd_fd;
static	array_list_t	inodes;
static	ppmo_t			p_read_pmo;
static	u8				p_read_buf;

static	void	kdriver_main(u32 thread_id, void* p_null);
static	bool	dispatch_message(pmsg_t p_msg);
static	void	on_open(pmsg_t p_msg);
static	void	on_read(pmsg_t p_msg);
static	void	on_write(pmsg_t p_msg);
static	void	on_readdir(pmsg_t p_msg);
static	void	on_access(pmsg_t p_msg);
static	void	on_stat(pmsg_t p_msg);
static	void	on_close(pmsg_t p_msg);

static	void		analyse_inodes();
static	u32			create_volume(u32 dev_num);
static	pinode_t	create_inode();
static	void		add_dirent(char* name, u32 inode, char* path);
static	u32			get_inode(char* path);
static	bool		checksum(ptar_record_header_t p_head);
static	u32			get_num(char* buf, size_t len);

void tarfs_init()
{
	k_status status;

	dbg_print("Initializing tarfs...\n");

	//Create heap
	fs_heap = mm_hp_create(4096, HEAP_EXTENDABLE | HEAP_MULTITHREAD);

	if(fs_heap == NULL) {
		excpt_panic(EFAULT,
		            "Failed to create tarfs heap!\n");
	}

	status = rtl_array_list_init(inodes, MAX_INODE_NUM, fs_heap);

	if(status != ESUCCESS) {
		excpt_panic(status, "Failed to initialize inode table of tarfs!\n");
	}

	//Create driver process
	if(pm_fork() == 0) {
		pm_exec("initrd_fs", NULL);
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

	//Create driver
	p_driver = vfs_create_drv_object("initrd_tarfs");
	p_driver->process_id = pm_get_crrnt_process();
	vfs_reg_driver(p_driver);
	tarfs_driver = p_driver->driver_id;

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
	switch(p_msg->message) {
	case MSG_OPEN:
		on_open(p_msg);
		break;

	case MSG_READ:
		on_read(p_msg);
		break;

	case MSG_WRITE:
		on_write(p_msg);
		break;

	case MSG_READDIR:
		on_readdir(p_msg);
		break;

	case MSG_ACCESS:
		on_access(p_msg);
		break;

	case MSG_STAT:
		on_stat(p_msg);
		break;

	case MSG_CLOSE:
		on_close(p_msg);
		break;

	default:
		msg_complete(p_msg, ENOTSUP);
	}

	return true;
}

void analyse_inodes()
{
	pinode_t p_inode;
	k_status status;
	pmsg_read_info_t p_info;
	pmsg_read_data_t p_data;
	u8*	p_block;
	ptar_record_header_t p_head;
	u32 tar_mode;
	char* p_name;
	u32 i, block_num;
	size_t size;

	//Create root inode
	p_inode = create_inode();

	status = pm_get_errno();

	if(statuc != ESUCCESS) {
		excpt_panic(status,
		            "Failed to create first inode of initrd.\n");
	}

	status = rtl_array_list_init(&(p_inode->data.dir_entries),
	                             MAX_DIRENT_NUM,
	                             fs_heap);

	if(static != ESUCCESS) {
		excpt_panic(status,
		            "Failed to create first inode of initrd.\n");
	}

	p_inode->gid = 0;
	p_inode->uid = 0;
	p_inode->mode = S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH
	                | S_IFDIR;

	//Scan ramdisk
	p_read_pmo = mm_pmo_create(4096);

	if(p_read_pmo == NULL) {
		excpt_panic(pm_get_errno(),
		            "Failed to create read buffer!");
	}

	p_read_buf = mm_pmo_map(NULL, p_read_pmo, false);

	if(p_read_buf == NULL) {
		excpt_panic(pm_get_errno(),
		            "Failed to map read buffer!");
	}

	p_info = (pmsg_read_info_t)p_read_buf;
	p_data = (pmsg_read_data_t)p_read_buf;

	//Get inodes
	while(1) {
		//Read block
		p_info->len = TAR_BLOCK_SIZE;
		status = vfs_read(initrd_fd, p_read_pmo);

		if(status != ESUCCESS) {
			excpt_panic(status,
			            "Read initrd error!\n");
		}

		if(p_data->len == 0) {
			break;
		}

		//Analyse head
		p_head = (ptar_record_header_t)(&(p_data->data));

		if(p_head->header.name[0] == '\0'
		   && p_head->header.chksum[0] == '\0'
		   && p_head->header.gname[0] == '\0'
		   && p_head->header.uname[0] == '\0') {
			break;
		}

		if(!checksum(p_head)) {
			excpt_panic(EIO,
			            "Initrd has been broken!\n");
		}

		//Create inode
		p_inode = create_inode();
		status = pm_get_errno();

		if(statuc != ESUCCESS) {
			excpt_panic(status,
			            "Failed to create inode of initrd.\n");
		}

		p_inode->uid = get_num(p_head->header.uid, 8);
		p_inode->gid = get_num(p_head->header.gid, 8);
		tar_mode = get_num(p_head->header.mode, 8);

		p_inode->mode = (tar_mode & TAR_UREAD ? S_IRUSR : 0)
		                | (tar_mode & TAR_UEXEC ? S_IXUSR : 0)
		                | (tar_mode & TAR_GREAD ? S_IRGRP : 0)
		                | (tar_mode & TAR_GEXEC ? S_IXGRP : 0)
		                | (tar_mode & TAR_OREAD ? S_IROTH : 0)
		                | (tar_mode & TAR_OEXEC ? S_IXOTH : 0)
		                | (p_head->header.linkflag == TAR_LF_DIR ? S_IFDIR : 0);

		if(p_inode->mode & S_IFDIR) {
			//Directory
			status = rtl_array_list_init(&(p_inode->data.dir_entries),
			                             MAX_DIRENT_NUM,
			                             fs_heap);

			if(static != ESUCCESS) {
				excpt_panic(status,
				            "Failed to create inode of initrd.\n");
			}

		} else {
			//Normal file
			p_inode->data.file_info.offset = vfs_seek(initrd_fd, SEEK_CUR, 0);
			p_inode->data.file_info.len = get_num(p_head->header.size, 12);
		}

		//Add directry entery
		p_name = p_head->header.name + rtl_strlen(p_head->header.name);
		p_name--;

		while(1) {
			p_name--;

			if(p_name == p_head->header.name) {
				add_dirent(p_head->header.name, p_inode->inode_num, "");
				break;

			} else if(*p_name == '/') {
				p_name = '\0';
				add_dirent(p_name, p_inode->inode_num, p_head->header.name);
				break;
			}
		}

		//Jump to next block
		size = get_num(p_head->header.size, 12);

		for(i = 0, block_num = size / TAR_BLOCK_SIZE + (size % TAR_BLOCK_SIZE ? 1 : 0);
		    i < block_num;
		    i++) {
			p_data->len = TAR_BLOCK_SIZE;
			vfs_read(initrd_fd, p_read_pmo);
		}
	}

	return;
}

u32 create_volume(u32 dev_num)
{
	k_status status;

	//Open ramdisk device
	initrd_fd = get_initrd_fd();

	//Analyse inodes
	analyse_inodes();


	//Create volume
}

static	pinode_t	create_inode();
static	void		add_dirent(char* name, u32 inode, char* path);
static	u32			get_inode(char* path);
static	bool		checksum(ptar_record_header_t p_head);
static	u32			get_num(char* buf, size_t len);
