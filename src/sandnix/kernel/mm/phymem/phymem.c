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

#include "../../../../common/common.h"
#include "../../rtl/rtl.h"
#include "../../debug/debug.h"
#include "../../pm/pm.h"
#include "../heap/heap.h"
#include "phymem.h"

#define	PHY_INIT_BITMAP_NUM		(PHY_INIT_BITMAP_SIZE * 8)

list_t		phymem_list = NULL;
list_t		phymem_bitmap_list = NULL;
void*		phymem_heap;

static	u8			phymem_heap_buf[PHYMEM_HEAP_SIZE];
static	spinlock_t	alloc_lock;

static	bitmap_t	init_bitmap[PHY_INIT_BITMAP_SIZE];

static	void	print_phymem();
static	void	create_init_bitmap();

static	void		destructor(pphymem_obj_t p_this);
static	pkstring_t	to_string(pphymem_obj_t p_this);

void phymem_init()
{
	dbg_kprint("Testing physical memory...\n");
	//phymem_heap = mm_hp_create_on_buf(phymem_heap_buf, PHYMEM_HEAP_SIZE,
	//                                  HEAP_EXTENDABLE | HEAP_MULTITHREAD
	//                                  | HEAP_PREALLOC);
	phymem_heap = mm_hp_create_on_buf(phymem_heap_buf, PHYMEM_HEAP_SIZE,
	                                  HEAP_MULTITHREAD
	                                  | HEAP_PREALLOC);

	phymem_init_arch();

	print_phymem();

	pm_init_spn_lock(&alloc_lock);

	//Create initialize bitmap
	create_init_bitmap();
	UNREFERRED_PARAMETER(create_init_bitmap);
	return;
}

void phymem_manage_all()
{
	void* begin_address;
	plist_node_t p_mmap_node;
	plist_node_t p_bitmap_node;
	pphymem_bitmap_t p_phy_bitmap;
	pphymem_tbl_entry_t p_mem_tbl;
	size_t bitmap_size;

	//Get begining address
	p_bitmap_node = phymem_bitmap_list;
	begin_address = NULL;

	do {
		p_phy_bitmap = (pphymem_bitmap_t)(p_bitmap_node->p_item);

		if((size_t)begin_address < (size_t)(p_phy_bitmap->base) + p_phy_bitmap->num * 4096) {
			begin_address = (void*)((size_t)(p_phy_bitmap->base) + p_phy_bitmap->num * 4096);
		}

		p_bitmap_node = p_bitmap_node->p_next;
	} while(p_bitmap_node != phymem_bitmap_list);

	//Create bitmaps
	p_mmap_node = phymem_list;

	do {
		p_mem_tbl = (pphymem_tbl_entry_t)(p_mmap_node->p_item);

		if(p_mem_tbl->status == PHY_MEM_ALLOCATABLE
		   || p_mem_tbl->status == PHY_MEM_DMA) {
			if((size_t)(p_mem_tbl->base) + p_mem_tbl->size
			   > (size_t)begin_address) {
				//Create new bitmaps
				p_phy_bitmap = hp_alloc_mm(sizeof(phymem_bitmap_t), phymem_heap);
				ASSERT(p_phy_bitmap != NULL);

				if((size_t)(p_mem_tbl->base) < (size_t)begin_address) {
					p_phy_bitmap->base = begin_address;
					p_phy_bitmap->num = (p_mem_tbl->size -
					                     ((size_t)begin_address - (size_t)(p_phy_bitmap->base))) % 4096;

				} else {
					p_phy_bitmap->base = p_mem_tbl->base;
					p_phy_bitmap->num = p_mem_tbl->size / 4096;
				}

				p_phy_bitmap->avail_num = p_phy_bitmap->num;

				bitmap_size = p_phy_bitmap->num;

				if(bitmap_size % 8 == 0) {
					bitmap_size /= 8;

				} else {
					bitmap_size = bitmap_size / 8 + 1;
				}

				switch(p_mem_tbl->status) {
					case PHY_MEM_ALLOCATABLE:
						p_phy_bitmap->status = PHYMEM_BITMAP_NORMAL;
						break;

					case PHY_MEM_DMA:
						p_phy_bitmap->status = PHYMEM_BITMAP_DMA;
						break;
				}

				p_phy_bitmap->p_bitmap = hp_alloc_mm(bitmap_size,
				                                     phymem_heap);
				ASSERT(p_phy_bitmap != NULL);
				rtl_memset(p_phy_bitmap->p_bitmap, 0, bitmap_size);
				pm_acqr_spn_lock(&alloc_lock);
				rtl_list_insert_after(&phymem_bitmap_list,
				                      NULL,
				                      p_phy_bitmap,
				                      phymem_heap);
				pm_rls_spn_lock(&alloc_lock);

			}
		}

		p_mmap_node = p_mmap_node->p_next;
	} while(p_mmap_node != phymem_list);

	return;
}

pphymem_obj_t mm_phymem_alloc(size_t num, bool is_dma)
{
	pphymem_obj_t p_ret;
	plist_node_t p_bitmap_node;
	pphymem_bitmap_t p_bitmap;
	u32 i, j;

	pm_acqr_spn_lock(&alloc_lock);

	//Compute how many memory blocks needed and allocate memory
	p_bitmap_node = phymem_bitmap_list;
	p_ret = NULL;

	do {
		p_bitmap = (pphymem_bitmap_t)(p_bitmap_node->p_item);

		if(p_bitmap->avail_num >= num
		   && (is_dma
		       ? p_bitmap->status == PHYMEM_BITMAP_DMA
		       : p_bitmap->status == PHYMEM_BITMAP_NORMAL)) {
			for(i = 0; i < p_bitmap->num; i++) {
				if(rtl_bitmap_read(p_bitmap->p_bitmap, i) == 0) {
					for(j = i;
					    j < p_bitmap->num
					    && rtl_bitmap_read(p_bitmap->p_bitmap, j) == 0;
					    j++) {
						if(j - i + 1 == num) {
							//Create object
							p_ret = hp_alloc_mm(sizeof(phymem_obj_t), phymem_heap);
							ASSERT(p_ret != NULL);
							INIT_KOBJECT(p_ret, destructor, to_string);
							p_ret->mem_block.base = p_bitmap->base + i * 4096;
							p_ret->mem_block.num = num;

							//Allocate physical memory
							for(j = i; j - i < num; j++) {
								rtl_bitmap_write(p_bitmap->p_bitmap,
								                 j, 1);
							}

							goto _end;
						}
					}
				}

				i = j - 1;
			}


		}

		p_bitmap_node = p_bitmap_node->p_next;
	} while(p_bitmap_node != phymem_bitmap_list);

_end:
	pm_rls_spn_lock(&alloc_lock);
	return p_ret;
}

void print_phymem()
{
	plist_node_t p_node;
	pphymem_tbl_entry_t p_entry;
	char* type_str;

	p_node = phymem_list;

	dbg_kprint("Physical memory info:\n");
	dbg_kprint("%-12s%-12s%-12s%-12s\n", "Begin", "End", "Size", "Type");

	do {
		p_entry = (pphymem_tbl_entry_t)(p_node->p_item);

		switch(p_entry->status) {
			case PHY_MEM_ALLOCATABLE:
				type_str = "Available";
				break;

			case PHY_MEM_ALLOCATED:
				type_str = "Allocated";
				break;

			case PHY_MEM_RESERVED:
				type_str = "Reserved";
				break;

			case PHY_MEM_SYSTEM:
				type_str = "System";
				break;

			case PHY_MEM_BAD:
				type_str = "Bad";
				break;

			case PHY_MEM_DMA:
				type_str = "DMA";
				break;

			case PHY_MEM_DMA_ALLOCATED:
				type_str = "DMA Allocated";
				break;
		}

		dbg_kprint("%-12P%-12P%-12P%-12s\n",
		           p_entry->base,
		           (u8*)(p_entry->base) + p_entry->size - 1,
		           p_entry->size,
		           type_str);
		p_node = p_node->p_next;
	} while(p_node != phymem_list);

	return;
}

void create_init_bitmap()
{
	u64 bits_init_num;
	pphymem_bitmap_t p_phy_mem;
	pphymem_tbl_entry_t p_mem_tbl;
	plist_node_t p_mmap_node;

	rtl_memset(&init_bitmap, 0, sizeof(init_bitmap));

	bits_init_num = 0;
	p_mmap_node = phymem_list;

	//Scan memory map
	do {
		p_mem_tbl = (pphymem_tbl_entry_t)(p_mmap_node->p_item);

		if(p_mem_tbl->status == PHY_MEM_ALLOCATABLE
		   && p_mem_tbl->status == PHY_MEM_DMA) {
			//Allocate bitmap
			if(p_mem_tbl->size / 4096 <= PHY_INIT_BITMAP_NUM - bits_init_num) {
				p_phy_mem = hp_alloc_mm(sizeof(phymem_bitmap_t), phymem_heap);
				ASSERT(p_phy_mem != NULL);
				p_phy_mem->base = p_mem_tbl->base;
				p_phy_mem->num = p_mem_tbl->size / 4096;
				p_phy_mem->avail_num = p_phy_mem->num;
				p_phy_mem->p_bitmap = init_bitmap + bits_init_num;

				switch(p_mem_tbl->status) {
					case PHY_MEM_ALLOCATABLE:
						p_phy_mem->status = PHYMEM_BITMAP_NORMAL;
						break;

					case PHY_MEM_DMA:
						p_phy_mem->status = PHYMEM_BITMAP_DMA;
						break;
				}

				bits_init_num += p_phy_mem->num;
				bits_init_num += (bits_init_num % 8 ? 0 : 8 - bits_init_num % 8);
				rtl_list_insert_after(&phymem_bitmap_list,
				                      NULL,
				                      p_phy_mem,
				                      phymem_heap);

			} else {
				p_phy_mem = hp_alloc_mm(sizeof(phymem_bitmap_t), phymem_heap);
				ASSERT(p_phy_mem != NULL);
				p_phy_mem->base = p_mem_tbl->base;
				p_phy_mem->num = PHY_INIT_BITMAP_NUM - bits_init_num;
				p_phy_mem->avail_num = p_phy_mem->num;
				p_phy_mem->p_bitmap = init_bitmap + bits_init_num;

				switch(p_mem_tbl->status) {
					case PHY_MEM_ALLOCATABLE:
						p_phy_mem->status = PHYMEM_BITMAP_NORMAL;
						break;

					case PHY_MEM_DMA:
						p_phy_mem->status = PHYMEM_BITMAP_DMA;
						break;
				}

				rtl_list_insert_after(&phymem_bitmap_list,
				                      NULL,
				                      p_phy_mem,
				                      phymem_heap);
				break;
			}
		}

		p_mmap_node = p_mmap_node->p_next;
	} while(p_mmap_node != phymem_list);

	return;
}

void destructor(pphymem_obj_t p_this)
{
	plist_node_t p_bitmap_node;
	pphymem_bitmap_t p_bitmap;
	u32 bit, i;

	p_bitmap_node = phymem_bitmap_list;

	do {
		p_bitmap = (pphymem_bitmap_t)(p_bitmap_node->p_item);

		if((size_t)(p_bitmap->base) <= (size_t)(p_this->mem_block.base)
		   && (size_t)(p_bitmap->base) + p_bitmap->num * 4096
		   >= (size_t)(p_this->mem_block.base) + p_this->mem_block.num * 4096) {
			//Free physical memory
			for(bit = ((size_t)(p_this->mem_block.base) - (size_t)(p_bitmap->base)) / 4096, i = 0;
			    i < p_this->mem_block.num;
			    i++, bit++) {
				rtl_bitmap_write(p_bitmap->p_bitmap,
				                 bit,
				                 0);
			}

			break;
		}

		p_bitmap_node = p_bitmap_node->p_next;
	} while(p_bitmap_node != phymem_bitmap_list);

	//Free object
	mm_hp_free(p_this, phymem_heap);
	return;
}

pkstring_t to_string(pphymem_obj_t p_this)
{
	pkstring_t p_ret;

	p_ret = rtl_ksprintf(NULL, "Physical memory object %P:\n%-10s : %P%-10s : %P\n",
	                     p_this,
	                     "Base",
	                     p_this->mem_block.base,
	                     "Size",
	                     p_this->mem_block.num * 4096);

	return p_ret;
}
