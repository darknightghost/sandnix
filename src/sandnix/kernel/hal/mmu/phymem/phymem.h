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

#include "../../../core/rtl/rtl_defs.h"

#include "./phymem_defs.h"

#define MODULE_NAME	hal_mmu

void	PRIVATE(phymem_init)();

kstatus_t hal_mmu_phymem_alloc(
    void** p_addr,		//Start address
    address_t align,	//Align
    bool is_dma,		//DMA page
    size_t page_num);	//Num

void hal_mmu_phymem_free(
    void* addr);		//Address

size_t hal_mmu_get_phymem_info(
    pphysical_memory_info_t p_buf,	//Pointer to buffer
    size_t size);					//Buffer size

//If the platform need to edit the physical memory map, edit this function.
void	PRIVATE(archecture_phyaddr_edit)(plist_t p_phy_addr_list);

#undef	MODULE_NAME
