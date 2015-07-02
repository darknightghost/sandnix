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

#include "partition.h"
#include "../memory/memory.h"
#include "../exception/exception.h"

static	u32			get_logical_partion_info(u32 extended_lba, u32 disk_info, u8 partition,
        u32* p_start_lba, u32* p_sector_count, u8* p_type);

u32 get_partition_info(u8 disk, u8 partition, u32* p_start_lba, u32* p_sector_count, u8* p_type)
{
	u32 disk_info;
	u8* sector_buf;
	ppartition_table p_table;
	u32 count;
	u32 ret;
	disk_info = get_hdd_info(disk);

	if(disk_info & DEVICE_NOT_EXISTS) {
		return PARTITION_NOT_FOUND;
	}

	sector_buf = malloc(HDD_SECTOR_SIZE);

	if(sector_buf == NULL) {
		panic(EXCEPTION_NOT_ENOUGH_MEMORY);
	}

	//Read boot sector
	if(!hdd_read(disk_info, 0, 1, sector_buf)) {
		free(sector_buf);
		return PARTITION_NOT_FOUND;
	}

	p_table = (ppartition_table)(sector_buf + 0x01BE);

	if(partition < 4) {
		//The partition is a primary partition or extended partition
		p_table += partition;

		if((p_table->state != 0x00
		    && p_table->state != 0x80)
		   || p_table->partition_type == 00) {
			free(sector_buf);
			return PARTITION_NOT_FOUND;
		}

		*p_start_lba = p_table->start_lba;
		*p_sector_count = p_table->sector_count;
		*p_type = p_table->partition_type;

		if(p_table->partition_type == 0x05) {
			free(sector_buf);
			return PARTITION_EXTENDED;

		} else {
			free(sector_buf);
			return PARTITION_PRIMARY;
		}

	} else {
		//The partition is a logic partition
		//Look for extended partition
		for(count = 0; count < 4; count++, p_table++) {
			if((p_table->state != 0x00
			    && p_table->state != 0x80)
			   || p_table->partition_type == 00) {
				free(sector_buf);
				return PARTITION_NOT_FOUND;
			}

			if(p_table->partition_type == 0x05) {
				ret = get_logical_partion_info(p_table->start_lba, disk_info, partition,
				                               p_start_lba, p_sector_count, p_type);
				free(sector_buf);
				return ret;
			}
		}

		free(sector_buf);
		return PARTITION_NOT_FOUND;
	}
}

u32 get_logical_partion_info(u32 extended_lba, u32 disk_info, u8 partition,
                             u32* p_start_lba, u32* p_sector_count, u8* p_type)
{
	u32 count;
	u8* sector_buf;
	ppartition_table p_table;
	u32 current_sector;
	sector_buf = malloc(HDD_SECTOR_SIZE);

	if(sector_buf == NULL) {
		panic(EXCEPTION_NOT_ENOUGH_MEMORY);
	}

	//Read sectors
	current_sector = extended_lba;

	if(!hdd_read(disk_info, current_sector, 1, sector_buf)) {
		free(sector_buf);
		return PARTITION_NOT_FOUND;
	}

	count = 4;

	//Look for partitions
	while(1) {
		p_table = (ppartition_table)(sector_buf + 0x01BE);

		if(count >= partition) {
			*p_start_lba = p_table->start_lba;
			*p_sector_count = p_table->sector_count;
			*p_type = p_table->partition_type;
			free(sector_buf);
			return PARTITION_LOGIC;
		}

		p_table++;

		if(current_sector > p_table->start_lba) {
			free(sector_buf);
			return PARTITION_NOT_FOUND;
		}

		current_sector = p_table->start_lba;

		if(!hdd_read(disk_info, current_sector, 1, sector_buf)) {
			free(sector_buf);
			return PARTITION_NOT_FOUND;
		}
	}
}
