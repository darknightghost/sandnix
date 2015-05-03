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

#ifndef	PHYSICAL_MEM_H_INCLUDE
#define	PHYSICAL_MEM_H_INCLUDE

#include "../../../../../common/arch/x86/types.h"

#define	PHYSICAL_PAGE_SIZE		(4*1024)

#define	PHY_PAGE_RESERVED		0x01
#define	PHY_PAGE_USABLE			0x02
#define	PHY_PAGE_UNUSABLE			0x03
#define	PHY_PAGE_SYSTEM			0x04
#define	PHY_PAGE_ALLOCATED		0x10

#pragma pack(1)
typedef struct _e820_table {
	u32		base_addr;
	u32		len;
	u32		type;
} e820_table, *pe820_table;
#pragma pack()

void		setup_e820();
void*		get_empty_physical_address

#endif	//!	PHYSICAL_MEM_H_INCLUDE
