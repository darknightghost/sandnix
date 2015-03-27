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

#include "fs.h"
#include "ext2.h"
#include "../memory/memory.h"
#include "../string/string.h"

pfile open(char* path)
{
	pfile ret;
	char* p;
	u32 disk;
	u32 partition;
	u32 diskinfo;
	u32 partition_type;
	u8 fs_type;
	u32 start_lba;
	u32 sector_count;
	//Get disk
	p = path;

	if(*p == '(' && *(p + 1) == 'h' * (p + 2) = 'd') {
		p += 3;
	} else {
		return 0;
	}

	disk = 0;

	while(*p >= '0' && *p <= '9') {
		disk = (*p - '0') + disk * 10;
	}

	diskinfo = get_hdd_info(disk);

	if(diskinfo & DEVICE_NOT_EXISTS) {
		return NULL;
	}

	//Get partition
	if(*p != ',') {
		return NULL;
	}

	p++;
	partition = 0;

	while(*p >= '0' && *p <= '9') {
		partition = (*p - '0') + partition * 10;
	}

	p++;

	if(*p != '/') {
		return NULL;
	}

	partition_type = get_partition_info(disk, partition,
										&start_lba, &sector_count, &fs_type);

	if(partition_type & PARTITION_NOT_FOUND
	   || partition_type & PARTITION_EXTENDED) {
		return NULL;
	}

	//Open file
	ret = malloc(sizeof(file));
	ret->disk_info = diskinfo;
	ret->partition_lba = start_lba;
	ret->fs_type = fs_type;
	ret->pos = 0;

	if(fs_type == FS_TYPE_EXT2) {
		if(!ext2_open(ret, p)) {
			free(ret);
			return NULL;
		}
	} else {
		free(ret);
		return NULL;
	}

	return ret;
}

u32 read(pfile fp, u8* buf, size_t buf_len)
{
	if(fp->fs_type == FS_TYPE_EXT2) {
		return ext2_read(fp, buf, buf_len);
	}

	return 0;
}

s32 seek(pfile fp, s32 offset, u8 start_pos)
{
	s32 ret;

	if(start_pos == FILE_POS_BEGIN) {
		if(offset < 0) {
			fp->pos = 0;
			return 0;
		}

		if(fp->size < offset) {
			fp->pos = fp->size;
			return fp->size;
		} else {
			fp->pos = (u32)offset;
			return offset;
		}
	} else if(start_pos == FILE_POS_END) {
		if(offset > 0) {
			fp->pos = fp->size;
			return 0;
		}

		if(fp->size < -offset) {
			fp->pos = 0;
			return -fp->size;
		} else {
			fp->pos = (size_t)(fp->size + offset);
			return offset;
		}
	} else if(start_pos == FILE_POS_CURRENT) {
		if(fp->pos + offset > fp->size) {
			ret = fp->size - fp->pos
				  fp->pos = fp->size;
			return ret;
		} else if(fp->pos + offset < 0) {
			ret = -fp->pos;
			fp->pos = 0;
			return ret;
		} else {
			fp->pos = (size_t)(fp->pos + offset);
			return offset;
		}
	}

	return 0;
}

void close(pfile fp)
{
	if(fp->fs_type == FS_TYPE_EXT2) {
		ext2_close(fp);
	}

	free(fp);
	return;
}
