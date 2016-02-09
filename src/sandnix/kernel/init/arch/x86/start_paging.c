/*
	  Copyright 2016,暗夜幽灵 <darknightghost.cn@gmail.com>

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

#include "../../../../../common/common.h"
#include "../../../../boot/multiboot2.h"
#include "../../init.h"
#include "../../../mm/mm.h"
#include "../../../io/io.h"

#define	KERNEL_PDE_INDEX	(KERNEL_MEM_BASE / (4096 * 1024))

static	void	get_needed_pages(u32 offset, void* p_boot_info,
                                 void** p_base,
                                 u32* p_num);

static	bool	is_apic_supported();

void start_paging(u32 offset, u32 magic, void* p_boot_info)
{
	void* base;
	u32 page_num;
	u32 i;
	ppte_t p_pte;
	void* page_table_base;
	ppde_t p_pde;
	u32 pde_num;
	u32 pte_num;
	void* apic_phy_base;

	get_needed_pages(offset, p_boot_info, &base,
	                 &page_num);

	page_table_base = (void*)((u32)base + 4096);

	//Ptes
	for(i = 0, p_pte = (ppte_t)page_table_base;
	    i < page_num;

	    i++, p_pte++) {
		p_pte->present = PG_P;
		p_pte->read_write = PG_RW;
		p_pte->user_supervisor = PG_SUPERVISOR;
		p_pte->write_through = PG_WRITE_THROUGH;
		p_pte->cache_disabled = 0;
		p_pte->accessed = 0;
		p_pte->dirty = 0;
		p_pte->page_table_attr_index = 0;
		p_pte->global_page = 1;
		p_pte->avail = PG_NORMAL;
		p_pte->page_base_addr = (i * 4096) >> 12;
	}


	if(is_apic_supported()) {
		__asm__ __volatile__(
		    "rdmsr\n"
		    "btsl	$11,%%eax\n"
		    "andl	$0xfffff000,%%eax\n"
		    :"=a"(apic_phy_base)
		    ::"dx");
		p_pte->present = PG_P;
		p_pte->read_write = PG_RW;
		p_pte->user_supervisor = PG_SUPERVISOR;
		p_pte->write_through = PG_WRITE_THROUGH;
		p_pte->cache_disabled = 0;
		p_pte->accessed = 0;
		p_pte->dirty = 0;
		p_pte->page_table_attr_index = 0;
		p_pte->global_page = 1;
		p_pte->avail = PG_NORMAL;
		p_pte->page_base_addr = ((u32)apic_phy_base) >> 12;

		*(u32*)(((u32)&apic_base_addr) + offset) = i * 4096 + KERNEL_MEM_BASE;
	}

	for(i = 0, p_pte = (ppte_t)page_table_base;
	    i < page_num;
	    i++, p_pte++) {
		p_pte->present = PG_P;
		p_pte->read_write = PG_RW;
		p_pte->user_supervisor = PG_SUPERVISOR;
		p_pte->write_through = PG_WRITE_THROUGH;
		p_pte->cache_disabled = 0;
		p_pte->accessed = 0;
		p_pte->dirty = 0;
		p_pte->page_table_attr_index = 0;
		p_pte->global_page = 1;
		p_pte->avail = PG_NORMAL;
		p_pte->page_base_addr = (i * 4096) >> 12;
	}

	if(i % 4096 > 0) {
		pte_num = 4096 - i % 4096;

		for(; i < pte_num;
		    i++, p_pte++) {
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
		}
	}

	//Pdes
	pde_num = i / 1024;

	if(i % 1024 > 0) {
		pde_num++;
	}

	for(i = 0, p_pde = (ppde_t)base;
	    i < 1024;
	    i++, p_pde++) {
		p_pde->read_write = PG_RW;
		p_pde->user_supervisor = PG_SUPERVISOR;
		p_pde->write_through = PG_WRITE_THROUGH;
		p_pde->cache_disabled = PG_ENCACHE;
		p_pde->accessed = 0;
		p_pde->reserved = 0;
		p_pde->page_size = PG_SIZE_4K;
		p_pde->global_page = 0;
		p_pde->avail = PG_NORMAL;

		if(i < pde_num) {
			p_pde->present = PG_P;
			p_pde->page_table_base_addr = (i * 4096
			                               + (u32)page_table_base) >> 12;

		} else if(i >= KERNEL_PDE_INDEX
		          && i < KERNEL_PDE_INDEX + pde_num) {
			p_pde->present = PG_P;
			p_pde->page_table_base_addr = ((i - KERNEL_PDE_INDEX) * 4096
			                               + (u32)page_table_base) >> 12;

		} else {
			p_pde->present = PG_NP;
			p_pde->page_table_base_addr = 0;
		}
	}

	//Start paging
	__asm__ __volatile__(
	    //Prepare for CR3
	    "movl	%0,%%eax\n"
	    "andl	$0xFFFFF000,%%eax\n"
	    "orl	$0x008,%%eax\n"
	    "movl	%%eax,%%cr3\n"
	    //Start paging
	    "movl	%%cr0,%%eax\n"
	    //Set CR0.PG
	    "orl	$0x80010000,%%eax\n"
	    "movl	%%eax,%%cr0\n"
	    ::"m"(base));

	UNREFERRED_PARAMETER(magic);
	return;
}

void get_needed_pages(u32 offset, void* p_boot_info,
                      void** p_base, u32* p_num)
{
	void* p_current;
	u32 boot_info_size;
	pmultiboot_tag_t p_tag;
	void* initrd_end;
	pmultiboot_tag_module_t p_module_info;
	u32 last_addr;
	pkernel_header_t p_kernel_header;
	u32 page_num;
	u32 i;

	//Boot info size
	p_current = p_boot_info;
	boot_info_size = *((u32*)p_current);
	p_current = (void*)((u32)p_current + sizeof(u32) * 2);

	initrd_end = 0;

	//Read tags to find the position of initrd & APIC memory
	while((u32)p_current - (u32)p_boot_info < boot_info_size) {
		p_tag = (pmultiboot_tag_t)p_current;

		if(p_tag->type == MULTIBOOT_TAG_TYPE_END) {
			break;
		}

		switch(p_tag->type) {
			case MULTIBOOT_TAG_TYPE_MODULE:
				p_module_info = (pmultiboot_tag_module_t)p_tag;
				initrd_end = (void*)p_module_info->mod_end;

				break;
		}

		p_current = (void*)((u32)p_current + p_tag->size);

		//Make the pointer 8-bytes aligned
		if((u32)p_current % 8 != 0) {
			p_current = (void*)((u32)p_current + (8 - (u32)p_current % 8));
		}
	}

	//Make the last_addr the end of all images
	last_addr = (u32)initrd_end;
	p_kernel_header = (pkernel_header_t)((u32)&kernel_header + offset);

	if(last_addr < p_kernel_header->data_end) {
		last_addr = p_kernel_header->data_end;
	}

	if(last_addr < p_kernel_header->text_end) {
		last_addr = p_kernel_header->text_end;
	}

	if(last_addr < (u32)p_boot_info + boot_info_size) {
		last_addr = (u32)p_boot_info + boot_info_size;
	}

	//4KB align
	if(last_addr % 4096 != 0) {
		last_addr += 4096 - last_addr % 4096;
	}

	*(void**)((u32)&kernel_address_offset + offset) = (void*)(offset);
	//Memory managment page
	*(void**)((u32)&mm_mgr_page_addr + offset) = (void*)(last_addr - offset);

	//Beging of the initialize page table
	last_addr += 4096;
	*(void**)((u32)&mm_init_pt_addr + offset) = (void*)(last_addr - offset);
	*p_base = (void*)last_addr;

	//Compute pages required to map
	//Pages required for 0 - last_addr
	page_num = last_addr / 4096;

	//APIC
	if(is_apic_supported()) {
		page_num++;
	}

	//Pages required for itself
	//PDT
	page_num += 1;

	//Page tables
	for(i = page_num / 1024; i > 0; i = i / 1024) {
		page_num += i;
	}

	*p_num = page_num;
	*(u32*)((u32)&init_page_num + offset) = page_num;

	return;
}

bool is_apic_supported()
{
	bool ret;

	__asm__ __volatile__(
	    "movl	$1,%%eax\n"
	    "cpuid\n"
	    "bt		$9,%%edx\n"
	    "setcb	%0\n"
	    :"=a"(ret)
	    ::"bx", "cx", "dx");

	return ret;
}
