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

#ifndef	FS_H_INCLUDE
#define	FS_H_INCLUDE

#include "../../rtl/rtl.h"
#include "../om/om.h"

#define	INVALID_FILEID		0xFFFFFFFF
#define	INVALID_FD			0xFFFFFFFF

struct	_mount_point_t;

typedef	struct	_path {
	struct _mount_point_t	*p_mount_point;
	char*					path;
} path_t, *ppath_t;

typedef	struct	_vfs_proc_info {
	path_t			root;
	path_t			pwd;
	u32				driver_obj;
	mutex_t			lock;
	array_list_t	file_descs;
} vfs_proc_info, *pvfs_proc_info;

typedef	struct _mount_point_t {
	path_t					path;
	u32						uid;
	u32						gid;
	u32						mode;
	u32						fs_dev;
	u32						volume_dev;
	struct	_mount_point_t*	p_parent;
	list_t					mount_points;
} mount_point_t, *pmount_point_t;

typedef	struct	_dirent {
	long		d_ino;		//Inode number
	size_t		d_off;		//Offset to this dirent
	size_t		d_reclen;	//length of this d_name
	char		d_name;		//filename (null-terminated)
} dirent_t, *pdirent_t;

typedef struct _file_stat_t {
	u32			dev_num;	//Device number of device containing file
	u32			inode;		//Inode number
	u32			mode;		//Protection
	u32			nlink;		//Number of hard links
	u32			uid;		//User ID of owner
	u32			gid;		//Group ID of owner
	u32			rdev;		//Device number (if special file)
	size_t		size;		//Total size, in bytes
	size_t		block_size;	//Blocksize for filesystem I/O
	u32			block_num;	//Number of 512B blocks allocated
	u32			atime;		//Time of last access
	u32			mtime;		//Time of last modification
	u32			ctime;		//Time of last status change
} file_stat_t, *pfile_stat_t;

typedef	struct {
	path_t		path;
	u32			file_obj;
	u64			offset;
	u32			flags;
	bool		serial_read;
} file_desc_t, *pfile_desc_t;

typedef	struct {
	u32		process_id;
	u32		fd;
} file_obj_ref_t, *pfile_obj_ref_t;

typedef	struct _file_obj	_file_obj_t, *pfile_obj_t;

void			fs_init();
k_status		add_file_obj(pfile_obj_t p_file_obj);
void			remove_file_obj(pfile_obj_t p_file_obj);
void			send_file_obj_destroy_msg(pfile_obj_t p_file_obj);
pfile_obj_t		get_file_obj(u32 id);
void			set_drv_obj(u32 driver_id);
bool			has_drv_object();
u32				get_initrd_fd();

#endif	//!	FS_H_INCLUDE
