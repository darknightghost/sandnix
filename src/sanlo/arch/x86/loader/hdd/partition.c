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

bool get_partition_info(u8 disk,u8 partition,u8* offset,u8* size)
{
	u32 disk_info;
	u8* sector_buf;
	u8* p;
	
	disk_info=get_hdd_info(disk);
	
	if(disk_info&DEVICE_NOT_EXISTS){
		return false;
	}
	
	sector_buf=malloc(HDD_SECTOR_SIZE);
	
	//Read boot sector
	hdd_read(disk_info,0,1,sector_buf);
}