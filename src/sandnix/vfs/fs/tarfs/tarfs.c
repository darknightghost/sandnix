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
static	hash_table_t	path_table;
static	array_list_t	file_id_table;

static	void	kdriver_main(u32 thread_id, void* p_null);
static	bool	dispatch_message(pmsg_t p_msg);
static	void	on_open(pmsg_t p_msg);
static	void	on_read(pmsg_t p_msg);
static	void	on_readdir(pmsg_t p_msg);
static	void	on_access(pmsg_t p_msg);
static	void	on_stat(pmsg_t p_msg);
static	void	on_close(pmsg_t p_msg);

static	void				analyse_inodes();
static	u32					create_volume(u32 dev_num);
static	pinode_t			create_inode();
static	void				add_dirent(char* name, u32 inode, char* path);
static	pinode_t			get_inode(char* path);
static	bool				checksum(ptar_record_header_t p_head);
static	u32					get_num(char* buf, size_t len);
static	u32					path_hash(char* path);
static	bool				path_comparer(char* path1, char* path2);
static	pfile_obj_info_t	get_fileobj_info(char* path, u32 mode, u32 process);

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

	status = rtl_array_list_init(file_id_table, MAX_FILEOBJ_NUM, fs_heap);

	if(status != ESUCCESS) {
		excpt_panic(status, "Failed to initialize file id table of tarfs!\n");
	}

	status = rtl_hash_table_init(&path_table, 0, 0xFFFF, (hash_func_t)path_hash,
	                             (compare_func_t)path_comparer, fs_heap);

	if(status != ESUCCESS) {
		excpt_panic(status, "Failed to initialize path hash table of tarfs!\n");
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

	case MSG_UNLINK:
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

	case MSG_SYNC:
		msg_complete(p_msg, ESUCCESS);
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
	u32 volume_dev;
	pdevice_obj_t p_device;

	//Open ramdisk device
	initrd_fd = get_initrd_fd();

	//Analyse inodes
	analyse_inodes();

	//Create volume
	p_device = vfs_create_dev_object("init_volume");
	p_device->gid = 0;
	p_device->device_number = MK_DEV(vfs_get_dev_major_by_name("volume",
	                                 DEV_TYPE_CHAR),
	                                 0);
	p_device->block_size = 1;
	vfs_add_device(p_device, p_driver->driver_id);
	volume_dev = p_device->device_number;

	return volume_dev;
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
	pdirent_t p_dirent, p_prev_dirent;;
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

	if(index == 0) {
		p_dirent->d_off = 0;

	} else {
		p_prev_dirent = rtl_array_list_get(&(p_dir_inode->data.dir_entries),
		                                   index - 1);
		ASSERT(p_prev_dirent != NULL);
		p_dirent->d_off = p_prev_dirent->d_off
		                  + sizeof(dirent_t) + p_prev_dirent->d_reclen - 1;
	}

	p_dirent->d_ino = inode;
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

u32 path_hash(char* path)
{
	u32 hash = 0;
	u32 i;
	u32 len;

	len = rtl_strlen(path);

	for(i = 0; i < len; i++) {
		hash = 33 * hash + *(path + i);
	}

	return hash % 0x00010000;
}

bool path_comparer(char* path1, char* path2)
{
	if(rtl_strcmp(path1, path2) == 0) {
		return true;
	}

	return false;
}

pfile_obj_info_t get_fileobj_info(char* path, u32 mode, u32 process)
{
	pfile_obj_info_t p_info;
	pinode_t p_inode;
	u32 uid;
	u32 gid;
	k_status status;
	size_t len;

	//Get file object
	p_info = rtl_hash_table_get(&path_table, path);

	if(p_info != NULL) {
		return p_info;
	}

	//Get inode
	p_inode = get_inode(path);

	if(p_inode == NULL) {
		pm_set_errno(ENOENT);
		return NULL;
	}

	//Check privilege
	if(mode & O_CREAT) {
		pm_set_errno(EROFS);
		return NULL;
	}

	if((mode & O_WRONLY) || (mode & O_RDWR)) {
		pm_set_errno(EROFS);
		return NULL;
	}

	if((mode & O_DIRECTORY) && !(p_inode->mode & S_IFDIR)) {
		pm_set_errno(ENOTDIR);
		return NULL;
	}

	if((!(mode & O_DIRECTORY)) && (p_inode->mode & S_IFDIR)) {
		pm_set_errno(EISDIR);
		return NULL;
	}

	if(!pm_get_proc_euid(process, &uid)) {
		pm_set_errno(ESRCH);
		return NULL;
	}

	if(!pm_get_proc_egid(process, &gid)) {
		pm_set_errno(ESRCH);
		return NULL;
	}

	if(p_inode->uid == uid) {
		//Owner
		if(mode & O_RDONLY) {
			if(!(p_inode->mode & S_IRUSR)) {
				pm_set_errno(EACCES);
				return NULL;
			}
		}

		if(mode & O_EXCL) {
			if(!(p_node->mode & S_IXUSR)) {
				pm_set_errno(EACCES);
				return NULL;
			}
		}

	} else if(p_inode->gid == gid) {
		//Group
		if(mode & O_RDONLY) {
			if(!(p_inode->mode & S_IRGRP)) {
				pm_set_errno(EACCES);
				return NULL;
			}
		}

		if(mode & O_EXCL) {
			if(!(p_node->mode & S_IXGRP)) {
				pm_set_errno(EACCES);
				return NULL;
			}
		}

	} else {
		//Others
		if(mode & O_RDONLY) {
			if(!(p_inode->mode & S_IROTH)) {
				pm_set_errno(EACCES);
				return NULL;
			}
		}

		if(mode & O_EXCL) {
			if(!(p_node->mode & S_IXOTH)) {
				pm_set_errno(EACCES);
				return NULL;
			}
		}
	}

	//Create file object
	p_info = mm_hp_alloc(sizeof(file_obj_info_t), fs_heap);

	if(p_info == NULL) {
		pm_set_errno(EFAULT);
		return NULL;
	}

	len = rtl_strlen(path) + 1;
	p_info->path = mm_hp_alloc(len, fs_heap);

	if(p_info->path == NULL) {
		mm_hp_free(p_info);
		pm_set_errno(EFAULT);
		return NULL;
	}

	rtl_strcpy_s(p_info->path, len, path);

	p_info->file_id == vfs_create_file_object();
	status = pm_get_errno();

	if(status != ESUCCESS) {
		mm_hp_free(p_info->path, fs_heap);
		mm_hp_free(p_info, fs_heap);
		pm_set_errno(status);
		return NULL;
	}

	p_info->p_inode = p_inode;

	rtl_hash_table_set(&path_table, p_info->path, p_info, fs_heap);
	ASSERT(OPERATE_SUCCESS);
	rtl_array_list_set(&file_id_table, p_info->file_id, p_info, fs_heap);
	ASSERT(OPERATE_SUCCESS);

	pm_set_errno(ESUCCESS);
	return p_info;
}

void on_open(pmsg_t p_msg)
{
	pmsg_open_info_t p_info;
	pfile_obj_info_t p_fo_info;
	k_status status;

	if(!(p_msg->flags.properties.pmo_buf)) {
		msg_complete(p_msg, EINVAL);
		return;
	}

	//Map buffer
	p_info = mm_pmo_map(NULL, p_msg->buf.pmo_addr, false);

	if(p_info == NULL) {
		msg_complete(p_msg, pm_get_errno());
		return;
	}

	//Get file object
	p_fo_info = get_fileobj_info(&(p_info->path), p_info->mode, p_info->process);

	if(p_fo_info == NULL) {
		status = pm_get_errno();
		mm_pmo_unmap(p_info, p_msg->buf.pmo_addr);
		msg_complete(p_msg, status);
		return;
	}

	p_info->file_object = p_fo_info->file_id;

	if(p_info->mode & O_DIRECTORY) {
		p_info->file_size = rtl_array_list_item_num(&p_inode->data.dir_entries);

	} else {
		p_info->file_size = p_fo_info->p_inode->data.file_info.len;
	}

	p_info->serial_read = false;

	mm_pmo_unmap(p_info, p_msg->buf.pmo_addr);
	msg_complete(p_msg, ESUCCESS);
	return;
}

void on_read(pmsg_t p_msg)
{
	pmsg_read_info_t p_info;
	pmsg_read_data_t p_data;
	k_status status;
	pfile_obj_info_t p_fileobj_info;

	if(!p_msg->flags.properties.pmo_buf) {
		msg_complete(p_msg, EINVAL);
		return;
	}

	//Map buffer
	p_info = mm_pmo_map(NULL, p_msg->buf.pmo_addr, false);

	if(p_info == NULL) {
		msg_complete(p_msg, pm_set_errno());
		return;
	}

	p_fileobj_info = rtl_array_list_get(&file_id_table, p_info->file_obj);

	if(p_fileobj_info == NULL) {
		status = pm_get_errno();
		mm_pmo_unmap(p_info, p_msg->buf.pmo_addr);
		msg_complete(p_msg, status);
		return;
	}

	if(p_fileobj_info->p_inode->mode & S_IFDIR) {
		mm_pmo_unmap(p_info, p_msg->buf.pmo_addr);
		msg_complete(p_msg, EISDIR);
		return;
	}

	if(p_info->offset >= p_fileobj_info->p_inode->data.file_info.len) {
		p_data = (pmsg_read_data_t)p_info;
		p_data->len = 0;
		mm_pmo_unmap(p_info, p_msg->buf.pmo_addr);
		msg_complete(p_msg, ESUCCESS);
		return;
	}

	//Forward the message
	if(p_info->len + p_info->offset > p_fileobj_info->p_inode->data.file_info.len) {
		p_info->len = p_fileobj_info->p_inode->data.file_info.len
		              - p_info->offset;
	}

	p_info->offset += p_fileobj_info->p_inode->data.file_info.offset;

	mm_pmo_unmap(p_info, p_msg->buf.pmo_addr);
	vfs_msg_forward(p_msg, initrd_ramdisk);
	return;
}

void on_readdir(pmsg_t p_msg)
{
	pmsg_readdir_info_t p_info;
	pmsg_read_data_t p_data;
	pfile_obj_info_t p_fo_info;
	pinode_t p_inode;
	u64 offset;
	size_t read_len;
	size_t count;
	pdirent_t p_dir_entry;
	u32 i;
	u8* p;
	u8* p_buf;
	size_t len;

	if(!p_msg->flags.properties.pmo_buf) {
		msg_complete(p_msg, EINVAL);
		return;
	}

	//Map buffer
	p_info = mm_pmo_map(NULL, p_msg->buf.pmo_addr, false);

	if(p_info == NULL) {
		msg_complete(p_msg, pm_get_errno());
		return;
	}

	p_data = (pmsg_read_data_t)p_info;

	//Look for inode
	p_fo_info = rtl_array_list_get(&file_id_table, p_info->file_obj);

	if(p_fo_info == NULL) {
		mm_pmo_unmap(p_info, p_msg->buf.pmo_addr);
		msg_complete(p_msg, EBADFD);
		return;
	}

	p_inode = p_fo_info->p_inode;

	if(!p_inode->mode & S_IFDIR) {
		mm_pmo_unmap(p_info, p_msg->buf.pmo_addr);
		msg_complete(p_msg, ENOTDIR);
		return;
	}

	//Read directory
	offset = p_info->offset;
	read_len = 0;
	count = p_info->count;
	p_buf = &(p_data->data);

	for(i = rtl_array_list_get_next_index(&(p_inode->data.dir_entries), 0);
	    OPERATE_SUCCESS;
	    i = rtl_array_list_get_next_index(&(p_inode->data.dir_entries), i + 1)) {
		p_dir_entry = rtl_array_list_get(&(p_inode->data.dir_entries), i);
		ASSERT(p_dir_entry != NULL);

		if(offset > 0) {
			//Offset
			if(offset >= sizeof(dirent_t) + p_dir_entry->d_reclen - 1) {
				offset -= sizeof(dirent_t) + p_dir_entry->d_reclen - 1;

			} else {
				p = (u8*)p_dir_entry;
				p += offset;
				len = sizeof(dirent_t) + p_dir_entry->d_reclen - 1 - offset;
				rtl_memcpy(&(p_data->data), p,
				           (count > len
				            ? len
				            : count));
				read_len = (count > len ? len : count);
				p_buf += offset;
				offset = 0;
			}

			continue;
		}

		if(read_len > count) {
			break;
		}

		//Read directory enteries
		if(sizeof(dirent_t) + p_dir_entry->d_reclen - 1 <= count - read_len) {
			rtl_memcpy(p_buf, p_dir_entry, sizeof(dirent_t) + p_dir_entry->d_reclen - 1);
			read_len += sizeof(dirent_t) + p_dir_entry->d_reclen - 1;

		} else {
			rtl_memcpy(p_buf, p_dir_entry, count - read_len);
			read_len = count;
		}

	}

	p_data->len = read_len;
	mm_pmo_unmap(p_info, p_msg->buf.pmo_addr);
	msg_complete(p_msg, ESUCCESS);

	return;
}

void on_access(pmsg_t p_msg)
{
	pmsg_access_info_t p_info;
	pinode_t p_inode;
	u32 uid;
	u32 gid;

	//Check buf type
	if(!p_msg->flags.properties.direct_buf) {
		msg_complete(p_msg, EINVAL);
		return;
	}

	p_info = (pmsg_access_info_t)(p_msg->buf.addr);

	//Get inode
	p_inode = get_inode(&(p_info->path));

	if(p_inode == NULL) {
		msg_complete(p_msg, ENOENT);
		return;
	}

	//Write
	if(p_info->mode & W_OK) {
		msg_complete(p_msg, EROFS);
		return;
	}

	if(!pm_get_proc_euid(p_info->process, &uid)) {
		msg_complete(p_msg, ESRCH);
		return;
	}

	if(!pm_get_proc_egid(p_info->process, &gid)) {
		msg_complete(p_msg, ESRCH);
		return;
	}

	//Read
	if(p_info->mode & R_OK) {
		if(uid == p_inode->uid) {
			if(!p_inode->mode & S_IRUSR) {
				msg_complete(p_msg, EACCES);
				return;
			}

		} else if(gid == p_inode->gid) {
			if(!p_inode->mode & S_IRGRP) {
				msg_complete(p_msg, EACCES);
				return;
			}

		} else {
			if(!p_inode->mode & S_IROTH) {
				msg_complete(p_msg, EACCES);
				return;
			}
		}
	}

	//Execute
	if(p_info->mode & W_OK) {
		if(uid == p_inode->uid) {
			if(!p_inode->mode & S_IWUSR) {
				msg_complete(p_msg, EACCES);
				return;
			}

		} else if(gid == p_inode->gid) {
			if(!p_inode->mode & S_IWGRP) {
				msg_complete(p_msg, EACCES);
				return;
			}

		} else {
			if(!p_inode->mode & S_IWOTH) {
				msg_complete(p_msg, EACCES);
				return;
			}
		}
	}

	msg_complete(p_msg, ESUCCESS);
	return;
}

void on_stat(pmsg_t p_msg)
{

}

void on_close(pmsg_t p_msg)
{

}
