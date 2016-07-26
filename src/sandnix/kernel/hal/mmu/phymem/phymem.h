/*
    Copyright 2016,王思远 <darknightghost.cn@gmail.com>

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

#pragma once

#include "../../../../../common/common.h"
#include "../../../core/rtl/rtl.h"

#define	PHYMEM_AVAILABLE	0x00
#define	PHYMEM_DMA			0x01
#define	PHYMEM_USED			0x02
#define	PHYMEM_DMA_USED		0x03
#define	PHYMEM_SYSTEM		0x04
#define	PHYMEM_RESERVED		0x05
#define	PHYMEM_BAD			0x06

typedef	struct	_physical_memory_info {
    u64			begin;
    u64			size;
    u32			type;
} physical_memory_info_t, *pphysical_memory_info_t;

void	phymem_init();

kstatus_t hal_mmu_phymem_alloc(
    void** p_addr,		//Start address
    bool is_dma,		//DMA page
    size_t page_num);	//Num

void hal_mmu_phymem_free(
    void* addr,			//Address
    size_t page_num);	//Num

size_t hal_mmu_get_phymem_info(
    pphysical_memory_info_t p_buf,	//Pointer to buffer
    size_t size);					//Buffer size

//If the platform need to edit the physical memory map, edit this function.
void	archecture_phyaddr_edit(plist_t p_phy_addr_list);
