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
#define	PHY_PAGE_UNUSABLE		0x03
#define	PHY_PAGE_SYSTEM			0x04
#define	PHY_PAGE_ALLOCATED		0x10

#define	E820_USABLE				0x01
#define	E820_RESERVED			0x02

#define	IS_DMA_MEM(a)			(((u32)(a))<1024*1024)

#pragma pack(1)
typedef struct _e820_table {
	u32		base_addr_l;
	u32		base_addr_h;
	u32		len_l;
	u32		len_h;
	u32		type;
} e820_table, *pe820_table;
#pragma pack()

void		init_phy_mem();
void*		get_physcl_page(void* base_addr, u32 num);
void*		get_dma_physcl_page(void* base_addr, u32 num);
void		free_physcl_page(void* base_addr, u32 num);
void		get_phy_mem_info(u32* phy_mem_num, u32* usable_num);

#endif	//!	PHYSICAL_MEM_H_INCLUDE
