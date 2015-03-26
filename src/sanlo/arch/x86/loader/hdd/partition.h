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

#ifndef	PARTION_H_INCLUDE
#define	PARTION_H_INCLUDE

#include "hdd.h"
#include "../types.h"

#define		PARTITION_NOT_FOUND		0
#define		PARTITION_PRIMARY		1
#define		PARTITION_EXTENDED		2
#define		PARTITION_LOGIC			3

#pragma	pack(1)
typedef	struct	_partition_table {
	u8		state;
	u8		begin_head;
	u16		begin_sector;
	u8		partition_type;
	u8		end_head;
	u16		end_sector;
	u32		start_lba;
	u32		sector_count;
} partition_table, *ppartition_table;
#pragma	pack()

u32		get_partition_info(u8 disk, u8 partition, u8* p_start_lba, u8* p_sector_count);

#endif
