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

#include "../../setup.h"

char	init_stack[4096];

void start_paging()
{
	u32 i;
	u32 mapped_size;
	ppde p_pde;
	ppte p_pte;

	//Initialize PTE
	//These pages should be mapped
	for(mapped_size = 0, p_pte = (ppte)TMP_PAGE_TABLE_BASE;
	    mapped_size < TMP_PAGED_MEM_SIZE;
	    mapped_size += 4 * 1024, p_pte++) {
		p_pte->present = PG_P;
		p_pte->read_write = PG_RW;
		p_pte->user_supervisor = PG_SUPERVISOR;
		p_pte->write_through = PG_WRITE_THROUGH;
		p_pte->cache_disabled = 0;
		p_pte->accessed = 0;
		p_pte->dirty = 0;
		p_pte->page_table_attr_index = 0;
		p_pte->global_page = 0;
		p_pte->avail = PG_NORMAL;
		p_pte->page_base_addr = mapped_size >> 12;
	}

	//These pages should not mapped
	while(p_pte < (ppte)(TMP_PAGE_TABLE_BASE + TMP_PAGE_SIZE)) {
		p_pte->present = PG_NP;
		p_pte->read_write = PG_RW;
		p_pte->user_supervisor = PG_SUPERVISOR;
		p_pte->write_through = PG_WRITE_THROUGH;
		p_pte->cache_disabled = 0;
		p_pte->accessed = 0;
		p_pte->dirty = 0;
		p_pte->page_table_attr_index = 0;
		p_pte->global_page = 0;
		p_pte->avail = PG_NORMAL;
		p_pte->page_base_addr = 0;
		p_pte++;
	}

	//Initialize PDE
	for(i = 0, p_pde = (ppde)TMP_PDT_BASE;
	    i < 1024;
	    i++, p_pde++) {
		if(i * 4096 * 1024 < TMP_PAGED_MEM_SIZE) {
			p_pde->present = PG_P;
			p_pde->page_table_base_addr = (i * 4096 + TMP_PAGE_TABLE_BASE) >> 12;

		} else if(i * 4096 * 1024 < TMP_PAGED_MEM_SIZE + KERNEL_MEM_BASE
		          && i * 4096 * 1024 >= KERNEL_MEM_BASE) {
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

	__asm__ __volatile__(
	    //Prepare for CR3
	    "movl	%0,%%eax\n\t"
	    "andl	$0xFFFFF000,%%eax\n\t"
	    "orl	$0x008,%%eax\n\t"
	    "movl	%%eax,%%cr3\n\t"
	    //Start paging
	    "movl	%%cr0,%%eax\n\t"
	    //Set CR0.PG & CR0.WP
	    "orl	$0x80010000,%%eax\n\t"
	    "movl	%%eax,%%cr0\n\t"
	    ::"i"(TMP_PDT_BASE));
	return;
}
