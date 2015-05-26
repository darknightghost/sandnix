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

#include "paging.h"
#include "../../../../setup/setup.h"
#include "../../../../rtl/rtl.h"
#include "../../../../debug/debug.h"
#include "../../../../exceptions/exceptions.h"

void*			pdt_table[MAX_PROCESS_NUM];
static	u32		current_pdt;

static	void*	kernel_mem_reserve(u32 base, u32 num);
static	void*	usr_mem_reserve(u32 base, u32 num);
static	void*	kernel_mem_commit(u32  base, u32 num);
static	void*	usr_mem_commit(u32 base, u32 num);

void paging_init()
{
	ppde p_pde;
	ppte p_pte;

	dbg_print("Initializing paging...\n");

	//Initialize PDT table
	rtl_memset(pg_table, 0, MAX_PROCESS_NUM + sizeof(void*));
	pdt_table[0] = TMP_PDT_BASE;

	//Initialize PDT of process 0
	dbg_print("Initializing  page table of process 0...\n");

	for(i = 0, p_pde = (ppde)(TMP_PDT_BASE + VIRTUAL_ADDR_OFFSET);
	    i < 1024;
	    i++, p_pde++) {
		if(i * 4096 * 1024 < TMP_PAGED_MEM_SIZE + VIRTUAL_ADDR_OFFSET
		   && i * 4096 * 1024 >= VIRTUAL_ADDR_OFFSET) {
			p_pde->present = PG_P;
			p_pde->page_table_base_addr = ((i - KERNEL_MEM_BASE / 4096 / 1024)
			                               * 4096 + TMP_PAGE_TABLE_BASE) >> 12;

		} else {
			p_pde->present = PG_NP;
			p_pde->page_table_base_addr = 0;
		}

		p_pde->read_write = PG_RW;
		p_pde->user_supervisor = PG_SUPERVISOR;
		p_pde->write_through = PG_WRITE_THROUGH;
		p_pde->cache_disabled = PG_ENCACHE;
		p_pde->accessed = 0;
		p_pde->reserved = 0;
		p_pde->page_size = PG_SIZE_4K;
		p_pde->global_page = 0;
		p_pde->avail = PG_NORMAL;
	}

	current_pdt = 0;

	//Refresh TLB
	REFRESH_TLB;

	return;
}

void* mm_virt_alloc(void* start_addr, size_t size, u32 options)
{
	void* base;
	u32 num;

	//Compute whitch page to allocate and how many pages to allocate
	base = (u32)start_addr / 4096;
	num = size / 4096 + size % 4096 ? 1 : 0;

	if(options & MEM_USER) {
		if(options & MEM_COMMIT) {
			return usr_mem_commit(base, num);

		} else if(options & MEM_RESERVE) {
			return usr_mem_reserve(base, num);
		}

	} else {
		if(options & MEM_COMMIT) {
			return kernel_mem_commit(base, num);

		} else if(options & MEM_RESERVE) {
			return kernel_mem_reserve(base, num);
		}
	}

	return NULL;
}

void* mm_virt_free(void* start_addr, size_t size, u32 options);
void* mm_virt_map(void* virt_addr, void* phy_addr);

u32 mm_pg_tbl_fork(u32 parent);
void mm_pg_tbl_free(u32 id);

void mm_pg_tbl_switch(u32 id)
{
	if(pdt_table[id] == NULL) {
		excpt_panic(EXCEPTION_ILLEGAL_PDT,
		            "An illegal id of page directory has been tied to switch to,the id is %p.",
		            id);
	}

	//Load CR3
	__asm__ __volatile__(
	    "movl	%0,%%eax\n\t"
	    "andl	$0xFFFFF000,%%eax\n\t"
	    "orl	$0x008,%%eax\n\t"
	    "movl	%%eax,%%cr3\n\t"
	    ::"m"(pdt_table[id]));

	return;
}

void mm_pg_tbl_usr_spc_clear(u32 id);

//Status
void mm_get_info(pmem_info p_info);


void* kernel_mem_reserve(u32 base, u32 num)
{
	//Check arguments
	if(base != NULL
	   && base < KERNEL_MEM_BASE) {
		return NULL;
	}

	return NULL;
}

void* usr_mem_reserve(u32 base, u32 num)
{
	//Check arguments
	if(base != NULL
	   && base < KERNEL_MEM_BASE) {
		return NULL;
	}

	return NULL;
}

void* kernel_mem_commit(u32 base, u32 num)
{
	return NULL;
}

void* usr_mem_commit(u32 base, u32 num)
{
	return NULL;
}

