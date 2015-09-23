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

#ifndef	FS_STRUCTS_H_INCLUDE
#define	FS_STRUCTS_H_INCLUDE

#include "../../../../common/common.h"
#include "../../../rtl/rtl.h"

typedef	struct	_inode {
	u32		uid;
	u32		gid;
	u32		mode;
	char*	file_name;
	union {
		struct {
			size_t			offset;
			size_t			len;
		} file_info;
		array_list_t	dir_entries;
	} data;
} inode_t, *pinode_t;

typedef	struct{
	u32				parent_dev;
	u32				volume_dev;
	u32				file_id;
	u32				thread_id;
	array_list_t	inodes;
}fs_volume_info_t,*pfs_volume_info_t;

#endif	//!	FS_STRUCTS_H_INCLUDE
