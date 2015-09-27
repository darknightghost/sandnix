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
static	void	on_readdir(pmsg_t p_msg);
static	void	on_access(pmsg_t p_msg);
static	void	on_stat(pmsg_t p_msg);
static	void	on_close(pmsg_t p_msg);

static	void		analyse_inodes();
static	u32			create_volume(u32 dev_num);
static	pinode_t	create_inode();
static	void		add_dirent(char* name, u32 inode, char* path);
static	pinode_t	get_inode(char* path);
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
		msg_complete(p_msg, EROFS);
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
		p_inode->mtime = get_num(p_head->header.mtime, 12);
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

pinode_t create_inode()
{
	u32 index;
	pinode_t p_inode;
	k_status status;

	index = rtl_array_list_get_free_index(&inodes);

	if(!OPERATE_SUCCESS) {
		excpt_panic(ENOMEM,
		            "Too much inodes!\n");
	}

	p_inode = mm_hp_alloc(sizeof(inode_t), fs_heap);

	if(p_inode == NULL) {
		excpt_panic(EFAULT,
		            "Failed to create inode!\n");
	}

	p_inode->inode_num = index;
	status = rtl_array_list_set(&inodes, index, p_inode, fs_heap);

	if(status != ESUCCESS) {
		excpt_panic(status,
		            "Failed to add inode!");
	}

	return p_inode;
}

void add_dirent(char* name, u32 inode, char* path)
{
	pinode_t p_dir_inode;
	pdirent_t p_dirent;
	u32 index;
	k_status status;

	//Get inode
	p_dir_inode = get_inode(path);

	if(p_dir_inode == NULL) {
		excpt_panic(pm_get_errno(),
		            "Failed to add dir entry!\n");
	}

	if(!p_dir_inode->mode & S_IFDIR) {
		excpt_panic(ENOTDIR,
		            "The file is note a directory!\n");
	}

	//Add directory entery
	index = rtl_array_list_get_free_index(&(p_dir_inode->data.dir_entries));

	if(!OPERATE_SUCCESS) {
		excpt_panic(pm_get_errno(),
		            "Failed to add dir entry!\n");
	}

	p_dirent = mm_hp_alloc(sizeof(dirent_t) + rtl_strlen(name),
	                       fs_heap);

	if(p_dirent == NULL) {
		excpt_panic(EFAULT,
		            "Failed to add dir entry!\n");
	}

	p_dirent->d_ino = inode;
	p_dirent->d_off = index;
	p_dirent->d_reclen = rtl_strlen(name) + 1;
	rtl_strcpy_s(&(p_dirent->d_name), p_dirent->d_reclen, name);

	status = rtl_array_list_set(&(p_dir_inode->data.dir_entries),
	                            index,
	                            p_dirent,
	                            fs_heap);

	if(status != ESUCCESS) {
		pm_set_errno(status,
		             "Failed to add dir entery!\n");
	}

	return;
}

pinode_t get_inode(char* path)
{
	char buf[NAME_MAX + 1];
	char* p;
	k_status status;
	u32 index;
	pinode_t p_inode;
	pdirent_t p_dir_entry;

	if(*path == '\0') {
		//Root inode
		return rtl_array_list_get(&inodes, 0);
	}

	p = path;
	p_inode = rtl_array_list_get(&inodes, 0);
	ASSERT(p_inode != NULL);

	for(status = rtl_get_next_name_in_path(&p, buf, NAME_MAX + 1);
	    *buf != '\0';
	    status = rtl_get_next_name_in_path(&p, buf, NAME_MAX + 1)) {

		if(status == EOVERFLOW) {
			break;

		} else if(status != ESUCCESS) {
			pm_set_errno(status);
			return NULL;
		}

		//Get inode
		if(!p_inode->mode & S_IFDIR) {
			pm_set_errno(ENOTDIR);
			return NULL;
		}

		for(index = rtl_array_list_get_next_index(&(p_inode->data.dir_entries),
		            0);
		    OPERATE_SUCCESS;
		    index = rtl_array_list_get_next_index(&(p_inode->data.dir_entries),
		            index + 1)) {
			p_dir_entry = rtl_array_list_get(&(p_inode->data.dir_entries),
			                                 index);
			ASSERT(p_dir_entry != NULL);

			if(rtl_strcmp(buf, &(p_dir_entry->d_name)) == 0) {
				//Found the inode
				p_inode = rtl_array_list_get(&inodes, p_dir_entry->d_ino);
				ASSERT(p_inode);
				goto _INODE_FOUND;
			}
		}

		//Inode not found
		pm_set_errno(ENOENT);
		return NULL;

_INODE_FOUND:
	}

	pm_set_errno(ESUCCESS);
	return p_inode;
}

bool checksum(ptar_record_header_t p_head)
{
	u32 sum;
	u32 sum_recorded;
	u32 i;

	sum_recorded = get_num(p_head->header.chksum, 8);
	rtl_memset(p_head->header.chksum, ' ', 8);

	for(i = 0, sum = 0;
	    i < sizeof(p_head->header);
	    i++) {
		sum += p_head->block[i];
	}

	if(sum_recorded != sum) {
		return false;

	} else {
		return true;
	}
}

u32 get_num(char* buf, size_t len)
{
	u32 ret;
	char* p;

	for(p = buf;
	    *p <= '0' && *p >= '7' && p - buf < len;
	    p++);

	for(ret = 0;
	    *p >= '0' && *p <= '7' && p - buf < len;
	    p++) {
		ret = ret * 8 + *p - '0';
	}

	return ret;
}
