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


#include "../types.h"
#include "../hdd/hdd.h"
#include "../hdd/partition.h"
#include "../memory/memory.h"
#include "../string/string.h"
#include "../io/stdout.h"
#include "ext2.h"

bool ext2_open(pfile fp, char* path)
{
	char buf[16];
	u32 super_block_lba;
	u32 block_size;
	pext2_file_info p_file_info;
	pext2_super_block p_super_block;
	
	//Get super block
	super_block_lba=fp->partition_lba+2;
	
	p_super_block=(pext2_super_block)malloc(EXT2_SUPER_BLOCK_SIZE);
	
	if(!hdd_read(fp->disk_info,
			super_block_lba,
			(u8)(EXT2_SUPER_BLOCK_SIZE/HDD_SECTOR_SIZE),
			(void*)p_super_block)){
		free(p_super_block);
		return false;
	}
	p_file_info=malloc(sizeof(ext2_file_info));
	block_size=1 << (p_super_block->s_log_block_size + 10);
	print_string(
			dectostr(block_size,buf),
			FG_BRIGHT_WHITE | BG_BLACK,
			BG_BLACK);
	return true;
	
}

u32 ext2_read(pfile fp, u8* buf, size_t buf_len)
{
	return 0;
}

void ext2_close(pfile fp)
{
	return;
}
