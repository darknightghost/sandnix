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

#ifndef	_ASM
#include "../../../core/mm/mm_defs.h"

#include "./paging_defs.h"

//Start paging
void		start_paging();

//Add more pages to initialize page table
void*		hal_mmu_add_early_paging_addr(void* phy_addr, u32 attr);

//Get kernel virtual address range
void		hal_mmu_get_krnl_addr_range(
    void** p_base,		//Pointer to basic address
    size_t* p_size);	//Pointer to size

//Get user virtual address range
void		hal_mmu_get_usr_addr_range(
    void** p_base,		//Pointer to basic address
    size_t* p_size);	//Pointer to size

//Create page table
kstatus_t	hal_mmu_pg_tbl_create(
    u32 id);		//Pointer to new page table id

//Destroy page table
void		hal_mmu_pg_tbl_destroy(
    u32 id);		//Page table id

//Set page table
kstatus_t	hal_mmu_pg_tbl_set(
    u32 id,
    void* virt_addr,				//Virtual address
    u32 attribute,					//Attribute
    void* phy_addr);				//Physical address

//Get mapping info
void hal_mmu_pg_tbl_get(
    u32 id,
    void* virt_addr,		//Virtual address
    void** phy_addr,		//Pointer to returned physical address
    u32* p_attr);			//Pointer to page attribute

//Refresh TLB
void hal_mmu_pg_tbl_refresh(void* virt_addr);

//Switch page table
void		hal_mmu_pg_tbl_switch(
    u32 id);			//Page table id

bool	hal_mmu_get_next_mapped_pages(
    void*		prev,		//Prev address
    void**		begin,		//Begining virtual address
    void**		p_phy_begin,//Pointer to begining physicall address.
    size_t*		p_size,		//Pointer to size
    u32*		p_attr);	//Pointer to attribute

void		paging_init();
address_t	get_load_offset();

#endif
