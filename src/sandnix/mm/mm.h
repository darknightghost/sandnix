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

#ifndef	MM_H_INCLUDE
#define	MM_H_INCLUDE

#include "../../common/common.h"

#ifdef	X86
	#include "paging/arch/x86/share.h"
	#include "paging/arch/x86/page_table.h"
	#include "paging/arch/x86/paging.h"
	#include "paging/arch/x86/physical_mem.h"
#endif	//X86

#ifdef X86

#include "heap/heap.h"

typedef	struct	{
	u32		phy_mem;
	u32		phy_mem_usable;
	u32		swap;
	u32		swap_usable;
	u32		pde_table_num;
	u32		pte_table_num;
	u32		shared_pages;
} mem_info_t, *pmem_info_t;

#endif	//X86

#define		MEM_USER			0x00000001
#define		MEM_COMMIT			0x00000002
#define		MEM_RESERVE			0x00000004
#define		MEM_UNCOMMIT		0x00000008
#define		MEM_RELEASE			0x00000010
#define		MEM_DMA				0x00000020

#define		PAGE_WRITEABLE		0x00000001
#define		PAGE_EXECUTABLE		0x00000002

#define		HEAP_EXTENDABLE				0x01
#define		HEAP_MULTITHREAD			0x02
#define		HEAP_DESTROY				0x04


void		mm_init();

//Physical memory
phy_page_state		mm_phy_mem_state_get(void* addr);

//Virtual memory
void*				mm_virt_alloc(void* start_addr, size_t size, u32 options, u32 attr);
void				mm_virt_free(void* start_addr, size_t size, u32 options);
void*				mm_virt_map(void* virt_addr, void* phy_addr);
void				mm_virt_unmap(void* virt_addr);

//Swap
void				mm_pg_lock(u32 id, void* address);
void				mm_pg_unlock(u32 id, void* address);

//Page table
u32					mm_pg_tbl_fork(u32 parent);
void				mm_pg_tbl_free(u32 pdt_id);
void				mm_pg_tbl_switch(u32 pdt_id);				//pm only
void				mm_pg_tbl_usr_spc_clear(u32 pdt_id);

//Status
void				mm_get_info(pmem_info_t p_info);

//Share memories
//PMO=Page mapping object
ppmo				mm_pmo_create(size_t size);
void				mm_pmo_free(ppmo p_pmo);
void*				mm_pmo_map(void* address, ppmo p_pmo, bool is_user);
void				mm_pmo_unmap(void* address, ppmo p_pmo);

//Heap
void*				mm_hp_create(size_t max_block_size, u32 attr);
void				mm_hp_destroy(void* heap_addr);
void*				mm_hp_alloc(size_t size, void* heap_addr);
void				mm_hp_chk(void* heap_addr);
void				mm_hp_free(void* addr, void* heap_addr);

#endif	//!	MM_H_INCLUDE

