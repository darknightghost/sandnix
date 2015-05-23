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

#ifdef	X86
	#include "../../common/arch/x86/types.h"
	#include "paging/arch/x86/page_table.h"
	#include "paging/arch/x86/paging.h"
	#include "paging/arch/x86/physical_mem.h"
	#include "heap/heap.h"
#endif	//X86

##ifdef X86

typedef	struct	{
	u32		phy_mem;
	u32		phy_mem_usable;
	u32		swap;
	u32		swap_usable;
	u32		pde_table_num;
	u32		pte_table_num;
	u32		shared_pages;
} mem_info, *pmem_info;

#endif	//X86

#define		MEM_USER			0x00000001
#define		MEM_COMMIT			0x00000002
#define		MEM_RESERVE			0x00000004
#define		MEM_RELEASE			0x00000008

void*		mm_virt_alloc(void* start_addr, size_t size, u32 options);
void*		mm_virt_free(void* start_addr, u32 options);
u32			mm_pg_tbl_fork(u32 parent);
void		mm_pg_tbl_free(u32 id);
void		mm_pg_tbl_switch(u32 id);
void		mm_get_info(pmem_info p_info);

void		mm_heap_create();
void		mm_heap_destroy();
void*		mm_heap_alloc(size_t size, void* heap_addr);
bool		mm_heap_chk(void* heap_addr);
void		mm_heap_free(void* addr, void* heap_addr);

void		mm_init();

#endif	//!	MM_H_INCLUDE

