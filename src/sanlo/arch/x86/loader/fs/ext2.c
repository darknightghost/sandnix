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

static	void		get_inode_offset(
	pext2_super_block p_super_block, pext2_group_desc p_group_desc,
	u32 block_size, u32 inode_num,
	/*out*/		u32* p_block, u32* p_offset);
static	u32			get_file_inode(pfile fp, pext2_inode p_parent_inode, u32 block_size, char* name, bool* is_directory);
static	bool		load_file_block(pfile fp, pext2_inode p_inode, u32 block);

bool ext2_open(pfile fp, char* path)
{
	char file_name[256];
	char* p;
	u32 super_block_lba;
	u32 inode_offset;
	pext2_file_info p_file_info;
	pext2_super_block p_super_block;
	char* block_buf;
	u32 group_num;
	pext2_group_desc p_group_desc;
	u32 group_desc_size;
	//Get super block
	super_block_lba = fp->partition_lba + 2;
	p_super_block = malloc(EXT2_SUPER_BLOCK_SIZE);
	u32 block;
	u32 offset;
	u32 next_inode;
	u32 inode_block_num;
	pext2_inode p_inode;
	bool directory_flag;

	if(p_super_block == NULL) {
		return false;
	}

	if(!hdd_read(fp->disk_info, super_block_lba,
				 EXT2_SUPER_BLOCK_SIZE / HDD_SECTOR_SIZE, p_super_block)) {
		free(p_super_block);
		return false;
	}

	if(p_super_block->s_magic != 0xEF53) {
		free(p_super_block);
		return false;
	}

	p_file_info = malloc(sizeof(ext2_file_info));

	if(p_file_info == NULL) {
		free(p_super_block);
		return false;
	}

	memset(p_file_info, sizeof(ext2_file_info), 0);
	//Compute block size
	p_file_info->block_size = 1 << (p_super_block->s_log_block_size + 10);
	block_buf = malloc(p_file_info->block_size);

	if(block_buf == NULL) {
		free(p_super_block);
		free(p_file_info);
		return false;
	}

	//Read GDT of block 0
	group_num = (p_super_block->s_blocks_count - p_super_block->s_first_data_block - 1)
				/ p_super_block->s_blocks_per_group + 1;
	group_desc_size = sizeof(ext2_group_desc) * group_num;
	p_group_desc = malloc((group_desc_size % p_file_info->block_size + 1)
						  * p_file_info->block_size);

	if(p_group_desc == NULL) {
		free(p_file_info);
		free(p_super_block);
		free(block_buf);
		return false;
	}

	if(!hdd_read(fp->disk_info,
				 fp->partition_lba + p_file_info->block_size / HDD_SECTOR_SIZE,
				 (u8)(((group_desc_size % p_file_info->block_size + 1)
					   * p_file_info->block_size) / HDD_SECTOR_SIZE),
				 (void*)block_buf)) {
		free(p_file_info);
		free(p_super_block);
		free(block_buf);
		return false;
	}

	p_file_info->p_group_desc = p_group_desc;
	//Get Root inode
	get_inode_offset(p_super_block, p_group_desc, block_size, 2, &block, &offset);

	if(!hdd_read(fp->disk_info,
				 fp->partition_lba + block * p_file_info->block_size / HDD_SECTOR_SIZE,
				 EXT2_SUPER_BLOCK_SIZE / HDD_SECTOR_SIZE,
				 (void*)p_group_desc)) {
		free(p_file_info);
		free(p_super_block);
		free(block_buf);
		return false;
	}

	p_inode = block_buf + offset;

	//Read directories
	if(*path != '\0') {
		p = path + 1;
	} else {
		p = path;
	}

	directory_flag = true;
	fp->extended_info = p_file_info;

	while(*p != '\0') {
		//Get next inode
		strcut(file_name, p, '/');
		p += strlen(file_name);

		if(*p = '/') {
			p++;
		}

		if(!directory_flag) {
			free(p_file_info);
			free(p_super_block);
			free(block_buf);
			return false;
		}

		next_inode = get_file_inode(fp, p_inode, p_file_info->block_size,
									&directory_flag, file_name);

		if(next_inode == 0) {
			(void*)p_group_desc)) {
				free(p_file_info);
				free(p_super_block);
				free(block_buf);
				return false;
			}
			get_inode_offset(p_super_block, p_group_desc, block_size, next_inode, &block, &offset);

			if(!hdd_read(fp->disk_info,
			fp->partition_lba + block * p_file_info->block_size / HDD_SECTOR_SIZE,
			EXT2_SUPER_BLOCK_SIZE / HDD_SECTOR_SIZE, (void*)p_group_desc)) {
				free(p_file_info);
				free(p_super_block);
				free(block_buf);
				return false;
			}
		}
	}

	memcpy(&(p_file_info->inode), p_inode, sizeof(ext2_file_info));
	/*print_string(
			dectostr(block_size,buf),
			FG_BRIGHT_WHITE | BG_BLACK,
			BG_BLACK);*/
	free(p_group_desc);
	free(p_super_block);
	free(block_buf);
	return true;
}

u32 ext2_read(pfile fp, u8 * buf, size_t buf_len)
{
	pext2_file_info p_file_info;
	p_file_info = fp->extended_info;
	return 0;
}

void ext2_close(pfile fp)
{
	pext2_file_info p_file_info;
	p_file_info = fp->extended_info;

	if(p_file_info->block_buf != NULL) {
		free(p_file_info->block_buf);
	}

	free(p_file_info);
	return;
}

void get_inode_offset(
	pext2_super_block p_super_block, pext2_group_desc p_group_desc,
	u32 block_size, u32 inode_num, u32 * p_block, u32 * p_offset)
{
	p_group_desc += (inode_num - 1) / p_super_block->s_inodes_per_group;
	*p_block = p_group_desc->bg_inode_table;
	*p_block += (inode_num - 1) % p_super_block->s_inodes_per_group
				* p_super_block->s_inode_size / block_size;
	*p_offset = (inode_num - 1) % p_super_block->s_inodes_per_group
				* p_super_block->s_inode_size % block_size;
	return;
}

u32 get_file_inode(pfile fp, pext2_inode p_parent_inode, u32 block_size, char* name, bool* is_directory)
{
	u32 current_block_num;
	char* block_buf;
	char* p;
	u32 read_len;
	u32 ret;
	pext2_dir_entry_2 p_dir_entry;
	p_dir_entry = malloc(sizeof(ext2_dir_entry));

	if(p_dir_entry == NULL) {
		return 0;
	}

	block_buf = malloc(block_size);

	if(block_buf == NULL) {
		free(p_dir_entry);
		return 0;
	}

	//Read dir entries
	read_len = 0;
	current_block_num = 0;
	p = block_buf;

	if(!load_file_block(fp, p_parent_inode, 0)) {
		free(p_dir_entry);
		free(block_buf);
		return 0;
	}

	while(read_len < p_parent_inode->i_size) {
		//Get dir entry
		if(block_size - (p - block_buf) < EXT2_DIR_ENTRY_BASIC_SIZE) {
			memcpy(p_dir_entry, p, block_size - (p - block_buf));
			current_block_num++;

			if(!load_file_block(fp, p_parent_inode, current_block_num)) {
				free(p_dir_entry);
				free(block_buf);
				return 0;
			}

			memcpy(p_dir_entry + (block_size - (p - block_buf)), block_buf,
				   EXT2_DIR_ENTRY_BASIC_SIZE - (block_size - (p - block_buf)));
			p = block_buf + (EXT2_DIR_ENTRY_BASIC_SIZE - (block_size - (p - block_buf)));
		} else {
			memcpy(p_dir_entry, p, EXT2_DIR_ENTRY_BASIC_SIZE);
			p += EXT2_DIR_ENTRY_BASIC_SIZE;
		}

		if(p - block_buf >= block_size) {
			current_block_num++;

			if(!load_file_block(fp, p_parent_inode, current_block_num)) {
				free(p_dir_entry);
				free(block_buf);
				return 0;
			}

			p = block_buf;
		}

		//Get dir name
		if(block_size - (p - block_buf) < p_dir_entry->rec_len - EXT2_DIR_ENTRY_BASIC_SIZE) {
			memcpy(p_dir_entry->name, p, block_size - (p - block_buf));
			current_block_num++;

			if(!load_file_block(fp, p_parent_inode, current_block_num)) {
				free(p_dir_entry);
				free(block_buf);
				return 0;
			}

			memcpy(p_dir_entry->name + (block_size - (p - block_buf)), block_buf,
				   p_dir_entry->rec_len - EXT2_DIR_ENTRY_BASIC_SIZE - (block_size - (p - block_buf)));
			p = block_buf + (p_dir_entry->rec_len - EXT2_DIR_ENTRY_BASIC_SIZE - (block_size - (p - block_buf)));
		} else {
			memcpy(p_dir_entry->name, p, p_dir_entry->rec_len - EXT2_DIR_ENTRY_BASIC_SIZE);
			p += p_dir_entry->rec_len - EXT2_DIR_ENTRY_BASIC_SIZE;
		}

		//Compare dir name
		if(strcmp(p_dir_entry->name, name) == 0) {
			ret = p_dir_entry->inode;
			if(p_dir_entry->file_type==EXT2_FT_DIR){
				*is_directory=true;
			}else{
				*is_directory=false;
			}
			free(p_dir_entry);
			free(block_buf);
			return ret;
		}

		if(p - block_buf >= block_size) {
			current_block_num++;

			if(!load_file_block(fp, p_parent_inode, current_block_num)) {
				free(p_dir_entry);
				free(block_buf);
				return 0;
			}

			p = block_buf;
		}

		read_len += p_dir_entry->rec_len;
	}

	free(p_dir_entry);
	free(block_buf);
	return 0;
}

bool load_file_block(pfile fp, pext2_inode p_inode, u32 block);
