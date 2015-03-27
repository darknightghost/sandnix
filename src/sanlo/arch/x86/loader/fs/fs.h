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

#include "../types.h"
#include "../hdd.h"
#include "../partition.h"

#define		FS_TYPE_EXT2		0x83

#define		FILE_TYPE_FILE		0
#define		FILE_TYPE_DIRECTORY	1

#define		FILE_POS_BEGIN		0
#define		FILE_POS_END		1
#define		FILE_POS_CURRENT	2

typedef	struct	_file {
	u32		disk_info;			//Which disk
	u32		partition_lba;		//Partition lba
	u8		fs_type;			//Type of file system
	u8		file_type;			//Type of file
	size_t	pos;				//Current position
	size_t	size;				//Size of file
	void*	extended_info;		//Used by file system functions
} file, *pfile;

pfile			open(char* path);
u32				read(pfile fp, u8* buf, size_t buf_len);
s32				seek(pfile fp, s32 offset, u8 start_pos);
void			close(pfile fp);



#endif	//! FS_H_INCLUDE
