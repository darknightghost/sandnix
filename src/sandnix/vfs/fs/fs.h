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

typedef	struct	_path {
	u32			volume_dev;
	char*		path;
} path_t, *ppath_t;

typedef	struct	_vfs_proc_info {
	path_t			root;
	path_t			pwd;
	u32				driver_obj;
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
} file_desc_t, *pfile_desc_t;

void			fs_init();
k_status		add_file_obj(pfile_obj_t file_obj);

#endif	//!	FS_H_INCLUDE
