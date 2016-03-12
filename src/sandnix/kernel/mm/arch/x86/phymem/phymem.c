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

static	bool			should_merge(pphymem_tbl_entry_t p1,
                                     pphymem_tbl_entry_t p2);
static	plist_node_t	merge_memory(plist_node_t p_node1,
                                     plist_node_t p_node2);

void phymem_init_arch()
{
	pphy_mem_info_t p_info;
	list_t memmap_list;
	plist_node_t p_node;
	pphymem_tbl_entry_t p_entry;
	pphymem_tbl_entry_t p_c_entry;
	pphymem_tbl_entry_t p_n_entry;
	pphymem_tbl_entry_t p_p_entry;
	bool sort_flag;
	void* t;

	memmap_list = init_get_phy_mem_info();
	p_node = memmap_list;

	do {
		p_info = (pphy_mem_info_t)(p_node->p_item);
		p_entry = mm_hp_alloc(sizeof(phymem_tbl_entry_t), phymem_heap);

		//Get memory type
		switch(p_info->type) {
			case BOOTINFO_MEMORY_AVAILABLE:
				p_entry->base = (void*)(u32)(p_info->start_addr);
				p_entry->size = p_info->size;
				p_entry->status = PHY_MEM_ALLOCATABLE;

				//4KB align the memory
				if((u32)(p_entry->base) % 4096 != 0) {
					p_entry->size -= 4096 - (u32)(p_entry->base) % 4096;
					p_entry->base = (void*)((u32)p_entry->base
					                        + (4096 - (u32)(p_entry->base) % 4096));
				}

				if(p_entry->size % 4096 != 0) {
					p_entry->size -= p_entry->size % 4096;
				}

				break;

			case BOOTINFO_MEMORY_RESERVED:
				p_entry->base = (void*)(u32)p_info->start_addr;
				p_entry->size = (size_t)p_info->size;
				p_entry->status = PHY_MEM_RESERVED;

				//4KB align the memory
				if((u32)(p_entry->base) % 4096 != 0) {
					p_entry->size += (u32)(p_entry->base) % 4096;
					p_entry->base = (void*)((u32)p_entry->base
					                        - (u32)(p_entry->base) % 4096);
				}

				if(p_entry->size % 4096 != 0) {
					p_entry->size += 4096 - p_entry->size % 4096;
				}

				break;

			case BOOTINFO_MEMORY_BADRAM:
				p_entry->base = (void*)(u32)p_info->start_addr;
				p_entry->size = p_info->size;
				p_entry->status = PHY_MEM_BAD;

				//4KB align the memory
				if((u32)(p_entry->base) % 4096 != 0) {
					p_entry->size += (u32)(p_entry->base) % 4096;
					p_entry->base = (void*)((u32)p_entry->base
					                        - (u32)(p_entry->base) % 4096);
				}

				if(p_entry->size % 4096 != 0) {
					p_entry->size += 4096 - p_entry->size % 4096;
				}

				break;
		}

		rtl_list_insert_after(&phymem_list, NULL, p_entry, phymem_heap);

		p_node = p_node->p_next;
	} while(p_node != memmap_list);

	//Sort the table
	sort_flag = true;

	while(sort_flag) {
		p_node = phymem_list;
		sort_flag = false;

		do {
			p_c_entry = (pphymem_tbl_entry_t)(p_node->p_item);
			p_n_entry = (pphymem_tbl_entry_t)(p_node->p_next->p_item);

			if((u32)(p_c_entry->base) > (u32)(p_n_entry->base)) {
				sort_flag = true;
				t = p_node->p_next->p_item;
				p_node->p_next->p_item = p_node->p_item;
				p_node->p_item = t;
			}

			p_node = p_node->p_next;
		} while(p_node != phymem_list->p_prev);
	}

	//Set unreferenced memory to reserved
	p_node = phymem_list;

	do {
		p_c_entry = (pphymem_tbl_entry_t)(p_node->p_item);
		p_n_entry = (pphymem_tbl_entry_t)(p_node->p_next->p_item);

		if((u32)(p_c_entry->base) + p_c_entry->size
		   < (u32)(p_n_entry->base)) {
			p_entry = mm_hp_alloc(sizeof(phymem_tbl_entry_t), phymem_heap);
			p_entry->base = (void*)((u32)(p_c_entry->base) + p_c_entry->size);
			p_entry->size = (u32)(p_n_entry->base) - (u32)(p_entry->base);
			p_entry->status = PHY_MEM_RESERVED;
			rtl_list_insert_after(&phymem_list, p_node,
			                      p_entry, phymem_heap);
			p_node = p_node->p_next->p_next;

		} else {
			p_node = p_node->p_next;
		}
	} while(p_node != phymem_list->p_prev);


	//The memory kernel used
	p_entry = mm_hp_alloc(sizeof(phymem_tbl_entry_t), phymem_heap);
	p_entry->base = (void*)(1024 * 1024);
	p_entry->size = init_page_num * 4096;
	p_entry->status = PHY_MEM_SYSTEM;
	rtl_list_insert_after(&phymem_list, NULL, p_entry, phymem_heap);

	//DMA memory
	p_entry = mm_hp_alloc(sizeof(phymem_tbl_entry_t), phymem_heap);
	p_entry->base = DMA_MEM_BASE;
	p_entry->size = DMA_MEM_SIZE;
	p_entry->status = PHY_MEM_DMA;
	rtl_list_insert_after(&phymem_list, NULL, p_entry, phymem_heap);

	//Sort the table
	sort_flag = true;

	while(sort_flag) {
		p_node = phymem_list;
		sort_flag = false;

		do {
			p_c_entry = (pphymem_tbl_entry_t)(p_node->p_item);
			p_n_entry = (pphymem_tbl_entry_t)(p_node->p_next->p_item);

			if((u32)(p_c_entry->base) > (u32)(p_n_entry->base)) {
				sort_flag = true;
				t = p_node->p_next->p_item;
				p_node->p_next->p_item = p_node->p_item;
				p_node->p_item = t;
			}

			p_node = p_node->p_next;
		} while(p_node != phymem_list->p_prev);
	}

	//Deal with overlapped memories
	p_node = phymem_list;

	do {
		//If the base address of next node is smaller than current node
		p_c_entry = (pphymem_tbl_entry_t)(p_node->p_item);
		p_n_entry = (pphymem_tbl_entry_t)(p_node->p_next->p_item);

		if((u32)(p_c_entry->base) > (u32)(p_n_entry->base)) {
			t = p_node->p_next->p_item;
			p_node->p_next->p_item = p_node->p_item;
			p_node->p_item = t;

		}

		if(p_node != phymem_list) {
			p_c_entry = (pphymem_tbl_entry_t)(p_node->p_item);
			p_p_entry = (pphymem_tbl_entry_t)(p_node->p_prev->p_item);
			p_n_entry = (pphymem_tbl_entry_t)(p_node->p_next->p_item);

			if((u32)(p_n_entry->base) == (u32)(p_p_entry->base) + p_p_entry->size
			   && p_p_entry->status == p_n_entry->status) {
				t = p_node->p_next->p_item;
				p_node->p_next->p_item = p_node->p_item;
				p_node->p_item = t;
				p_node = p_node->p_prev;

			}

			if(p_c_entry->base == p_p_entry->base) {
				p_node = p_node->p_prev;
			}
		}

		//Merge node
		if(should_merge((pphymem_tbl_entry_t)(p_node->p_item),
		                (pphymem_tbl_entry_t)(p_node->p_next->p_item))) {
			p_node = merge_memory(p_node, p_node->p_next);

		} else {
			p_node = p_node->p_next;
		}
	} while(p_node != phymem_list->p_prev);

	UNREFERRED_PARAMETER(should_merge);
	UNREFERRED_PARAMETER(merge_memory);
	return;
}

bool should_merge(pphymem_tbl_entry_t p1, pphymem_tbl_entry_t p2)
{
	if((u32)(p1->base) + p1->size > (u32)(p2->base)
	   || (p1->status == p2->status
	       && (u32)(p1->base) + p1->size == (u32)(p2->base))) {
		return true;
	}

	return false;
}

plist_node_t merge_memory(plist_node_t p_node1, plist_node_t p_node2)
{
	pphymem_tbl_entry_t p_entry1;
	pphymem_tbl_entry_t p_entry2;
	pphymem_tbl_entry_t p_new_entry;

	p_entry1 = (pphymem_tbl_entry_t)(p_node1->p_item);
	p_entry2 = (pphymem_tbl_entry_t)(p_node2->p_item);

	if(p_entry1->status < p_entry2->status) {
		//Overlapped region belongs to entry2
		if((u32)(p_entry1->base) + p_entry1->size
		   > (u32)p_entry2->base + p_entry2->size) {
			//Entry1 contains entry2
			p_new_entry = mm_hp_alloc(sizeof(phymem_tbl_entry_t), phymem_heap);
			p_new_entry->base = (void*)((u32)(p_entry2->base) + p_entry2->size);
			p_new_entry->size = ((u32)(p_entry1->base) + p_entry1->size)
			                    - ((u32)p_entry2->base + p_entry2->size);
			p_new_entry->status = p_entry1->status;
			p_entry1->size = (u32)(p_entry2->base) - (u32)(p_entry1->base);

			if(p_entry1->size == 0) {
				rtl_list_remove(&phymem_list, p_node1, phymem_heap);
				mm_hp_free(p_entry1, phymem_heap);
			}

			return rtl_list_insert_after(&phymem_list, p_node2,
			                             p_new_entry, phymem_heap);

		} else {
			p_entry1->size = (u32)(p_entry2->base) - (u32)(p_entry1->base);

			if(p_entry1->size == 0) {
				rtl_list_remove(&phymem_list, p_node1, phymem_heap);
				mm_hp_free(p_entry1, phymem_heap);
			}

			return p_node2;
		}


	} else if(p_entry1->status == p_entry2->status) {
		//Merge entry1 to entry2 and remove entry2
		p_entry2->size = (u32)(p_entry2->base) + p_entry2->size
		                 - (u32)(p_entry1->base);
		p_entry2->base = p_entry1->base;
		rtl_list_remove(&phymem_list, p_node1, phymem_heap);
		mm_hp_free(p_entry1, phymem_heap);
		return p_node2;

	} else {
		//Overlapped region belongs to entry1
		if((u32)(p_entry1->base) + p_entry1->size
		   > (u32)p_entry2->base + p_entry2->size) {
			//Entry1 contains entry2
			rtl_list_remove(&phymem_list, p_node2, phymem_heap);
			mm_hp_free(p_entry2, phymem_heap);
			return p_node1->p_next;

		} else {
			p_entry2->size = (u32)(p_entry2->base) + p_entry2->size
			                 - ((u32)(p_entry1->base) + p_entry1->size);

			if(p_entry2->size == 0) {
				rtl_list_remove(&phymem_list, p_node2, phymem_heap);
				mm_hp_free(p_entry2, phymem_heap);
				return p_node1->p_next;
			}

			p_entry2->base = (void*)((u32)(p_entry1->base) + p_entry1->size);
			return p_node2;
		}
	}
}
