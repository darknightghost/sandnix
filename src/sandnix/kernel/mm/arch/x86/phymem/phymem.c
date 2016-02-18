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

#include "../../../../../../common/common.h"
#include "../../../../init/init.h"
#include "../../../mm.h"
#include "../../../phymem/phymem.h"

void phymem_init_x86()
{
	pphy_mem_info_t p_info;
	list_t memmap_list;
	plist_node_t p_node;
	pphymem_tbl_entry_t p_entry;
	bool clean_flag;

	memmap_list = init_get_phy_mem_info();
	p_node = memmap_list;

	do {
		p_info = (pphy_mem_info_t)(p_node->p_item);
		p_entry = mm_hp_alloc(sizeof(phymem_tbl_entry_t), NULL);

		//Get memory type
		switch(p_info->type) {
			case BOOTINFO_MEMORY_AVAILABLE:
				p_entry->base = p_info->start_addr;
				p_entry->size = p_info->size;
				p_entry->status = PHY_MEM_ALLOCATABLE;

				//4KB align the memory
				if((u32)(p_entry->base) % 4096 != 0) {
					p_entry->size -= (u32)(p_entry->base) % 4096;
					p_entry->base = (void*)((u32)p_entry->base
					                        + (u32)(p_entry->base) % 4096);
				}

				if(p_entry->size % 4096 != 0) {
					p_entry->size -= 4096 - p_entry->size % 4096;
				}

				break;

			case BOOTINFO_MEMORY_RESERVED:
				p_entry->base = p_info->start_addr;
				p_entry->size = p_info->size;
				p_entry->status = PHY_MEM_RESERVED;

				//4KB align the memory
				if((u32)(p_entry->base) % 4096 != 0) {
					p_entry->size += 4096 - (u32)(p_entry->base) % 4096;
					p_entry->base = (void*)((u32)p_entry->base
					                        - (4096 - (u32)(p_entry->base) % 4096));
				}

				if(p_entry->size % 4096 != 0) {
					p_entry->size += p_entry->size % 4096;
				}

				break;

			case BOOTINFO_MEMORY_BADRAM:
				p_entry->base = p_info->start_addr;
				p_entry->size = p_info->size;
				p_entry->status = PHY_MEM_BAD;

				//4KB align the memory
				if((u32)(p_entry->base) % 4096 != 0) {
					p_entry->size += 4096 - (u32)(p_entry->base) % 4096;
					p_entry->base = (void*)((u32)p_entry->base
					                        - (4096 - (u32)(p_entry->base) % 4096));
				}

				if(p_entry->size % 4096 != 0) {
					p_entry->size += p_entry->size % 4096;
				}

				break;
		}

		rtl_list_insert_after(&phmem_list, NULL, p_entry, NULL);

		p_node = p_node->p_next;
	} while(p_node != memmap_list);

	//The memory kernel used
	p_entry = mm_hp_alloc(sizeof(phymem_tbl_entry_t), NULL);
	p_entry->base = (void*)(1024 * 1024);
	p_entry->size = init_page_num * 4096;
	p_entry->status = PHY_MEM_SYSTEM;
	rtl_list_insert_after(&phmem_list, NULL, p_entry, NULL);

	//Sort the memories and deal with overlapped memories
	clean_flag = false;

	while(!clean_flag) {
		clean_flag = true;
	}
}
