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

#define	IS_BACKUP_GROUP(num)	(!(((num)%3)*((num)%5)*((num)%7)))
#define	GET_INODE_OFFSET(num,inode_per_group,block_per_group,inode_size,group_num)	\
			(((((num)-1) / (inode_per_group)) * (block_per_group) + (1+(group_num)\
			+(group_num)+1+1) * IS_BACKUP_GROUP((num)/(inode_per_group)) \
			* (block_size) + (((num-1)) % (inode_per_group)) * (inode_size))

bool ext2_open(pfile fp, char* path)
{
	char buf[16];
	u32 super_block_lba;
	u32 inode_offset;
	pext2_file_info p_file_info;
	pext2_super_block p_super_block;
	char* block_buf;
	
	//Get super block
	super_block_lba=fp->partition_lba+2;
	
	p_super_block=malloc(EXT2_SUPER_BLOCK_SIZE);
	if(p_super_block==NULL){
		return false;
	}
	if(!hdd_read(fp->disk_info,
			super_block_lba,
			(u8)(EXT2_SUPER_BLOCK_SIZE/HDD_SECTOR_SIZE),
			(void*)p_super_block)){
		free(p_super_block);
		return false;
	}
	if(p_super_block->s_magic!=0xEF53){
		free(p_super_block);
		return false;
	}
	
	p_file_info=malloc(sizeof(ext2_file_info));
	if(p_file_info==NULL){
		free(p_super_block);
		return false;
	}
	memset(p_file_info,sizeof(ext2_file_info),0);
	
	//Compute block size
	p_file_info->block_size=1 << (p_super_block->s_log_block_size + 10);
	block_buf=malloc(p_file_info->block_size);
	
	if(block_buf==NULL){
		free(p_super_block);
		free(p_file_info);
		return false;
	}
	
	//Read GDT of block 0

	/*print_string(
			dectostr(block_size,buf),
			FG_BRIGHT_WHITE | BG_BLACK,
			BG_BLACK);*/
	free(p_super_block);
	free(block_buf);
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
