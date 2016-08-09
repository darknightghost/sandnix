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

#if defined X86
    #define	KERNEL_MEM_BASE		0xC0000000
    #define	KERNEL_MEM_SIZE		(1024 * 1024 * 1024)
    #include "./arch/x86/page_table.h"

#elif defined ARM_ARMV7
    #define	KERNEL_MEM_BASE		0xC0000000
    #define	KERNEL_MEM_SIZE		(1024 * 1024 * 1024)
    #include "./arch/arm/armv7/page_table.h"
#endif

#include "../../../core/mm/mm.h"

//Start paging
void		start_paging();

//Add more pages to initialize page table
void*		hal_mmu_add_early_paging_addr(void* phy_addr);

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
    u32* page_id);		//Pointer to new page table id

//Destroy page table
void		hal_mmu_pg_tbl_destroy(
    u32 page_id);		//Page table id

//Set page table
void		hal_mmu_pg_tbl_set(
    void* virt_addr,				//Virtual address
    u32 num,						//Count of pages
    pkrnl_pg_tbl_t page_tables);	//Kernel page table

//Refresh TLB
void		hal_mmu_pg_tbl_refresh();

//Switch page table
void		hal_mmu_pg_tbl_switch(
    u32 id);			//Page table id

address_t	get_load_offset();
