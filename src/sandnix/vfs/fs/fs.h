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

#include "../vfs.h"
#include "../../rtl/rtl.h"

#define	INVALID_FILEID		0xFFFFFFFF
#define	INVALID_FD			0xFFFFFFFF

typedef	struct	_path {
	u32			volume_dev;
	char*		path;
} path_t, *ppath_t;

typedef	struct	_vfs_proc_info {
	path_t			root;
	path_t			pwd;
	u32				driver_obj;
	mutex_t			lock;
	array_list_t	file_descs;
} vfs_proc_info, *pvfs_proc_info;

typedef	struct {
	path_t		path;
	u32			access;
	u32			fs_dev;
	u32			volume_dev;
	list_t		mount_points;
} mount_point_t, *pmount_point_t;

typedef	struct {
	path_t		path;
	u32			file_obj;
	size_t		offset;
	size_t		size;
	u32			flags;
} file_desc_t, *pfile_desc_t;

typedef	struct {
	u32		process_id;
	u32		fd;
} file_obj_ref_t, *pfile_obj_ref_t;

typedef	struct {
	char*	name;
	bool	multi_mount;
	bool	mounted;
} volume_info, *pvolume_info;

void			fs_init();
k_status		add_file_obj(pfile_obj_t p_file_obj);
void			remove_file_obj(pfile_obj_t p_file_obj);
void			send_file_obj_destroy_msg(pfile_obj_t p_file_obj);
pfile_obj_t		get_file_obj(u32 id);
void			set_drv_obj(u32 driver_id);
bool			has_drv_object();

#endif	//!	FS_H_INCLUDE
