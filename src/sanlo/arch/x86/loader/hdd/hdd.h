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

#ifndef	HDD_H_INCLUDE
#define	HDD_H_INCLUDE

#include "../io/io.h"
#include "../types.h"

#define	HDD_SECTOR_SIZE				512
#define	DEVICE_NOT_EXISTS			0x00000001
#define	DEVICE_MASTER_FLAG			0x00000002
#define	DEVICE_PORT_PRIMARY_FLAG	0x00000004

#pragma	pack(1)
typedef union _ide_device_reg {
	u8		value;
	struct {
		u8	lba_high: 4;
		u8	drv_slave_flag: 1;
		u8	always_1_1: 1;
		u8	lba_mode: 1;
		u8	always_1_2: 1;
	} members;
} ide_device_reg, *pide_device_reg;
#pragma	pack()

u32		get_hdd_info(u8 dev);
bool	hdd_read(u32 hdd_info, u32 start_sector, u8 sector_num, u8* buf);

#endif	//! HDD_H_INCLUDE
