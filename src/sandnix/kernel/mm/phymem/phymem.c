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

static	bool		alloc_phypages(u32 num);

static	void		destructor(pphymem_obj_t p_this);
static	pkstring_t	to_string(pphymem_obj_t p_this);

void phymem_init()
{
	dbg_kprint("Testing physical memory...\n");
	phymem_heap = mm_hp_create_on_buf(phymem_heap_buf, PHYMEM_HEAP_SIZE,
	                                  HEAP_EXTENDABLE | HEAP_MULTITHREAD);

	phymem_init_arch();

	print_phymem();

	pm_init_spn_lock(&alloc_lock);

	//Create initialize bitmap
	create_init_bitmap();
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

		if((size_t)begin_address < (size_t)(p_phy_bitmap->base) + p_phy_bitmap->size) {
			begin_address = (void*)((size_t)(p_phy_bitmap->base) + p_phy_bitmap->size);
		}

		p_bitmap_node = p_bitmap_node->p_next;
	} while(p_bitmap_node != phymem_bitmap_list);

	//Create bitmaps
	p_mmap_node = phymem_list;

	do {
		p_mem_tbl = (pphymem_tbl_entry_t)(p_mmap_node->p_item);

		if(p_mem_tbl->status == PHY_MEM_ALLOCATABLE) {
			if((size_t)(p_mem_tbl->base) + p_mem_tbl->size > (size_t)begin_address) {
				//Create new bitmaps
				p_phy_bitmap = mm_hp_alloc(sizeof(phymem_bitmap_t), phymem_heap);

				if((size_t)(p_mem_tbl->base) < (size_t)begin_address) {
					p_phy_bitmap->base = begin_address;
					p_phy_bitmap->size = p_mem_tbl->size -
					                     ((size_t)begin_address - (size_t)(p_phy_bitmap->base));

				} else {
					p_phy_bitmap->base = p_mem_tbl->base;
					p_phy_bitmap->size = p_mem_tbl->size;
				}

				bitmap_size = p_phy_bitmap->size / 4096;

				if(bitmap_size % 8 == 0) {
					bitmap_size /= 8;

				} else {
					bitmap_size = bitmap_size / 8 + 1;
				}

				p_phy_bitmap->p_bitmap = mm_hp_alloc(bitmap_size,
				                                     phymem_heap);
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

pphymem_obj_t mm_phymem_alloc(size_t size)
{
}

pphymem_obj_t mm_phymem_get_reserved(void* base, size_t size)
{
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

bool alloc_phypages(u32 num)
{
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

		if(p_mem_tbl->status == PHY_MEM_ALLOCATABLE) {
			//Allocate bitmap
			if(p_mem_tbl->size / 4096 <= PHY_INIT_BITMAP_NUM - bits_init_num) {
				p_phy_mem = mm_hp_alloc(sizeof(phymem_bitmap_t), phymem_heap);
				p_phy_mem->base = p_mem_tbl->base;
				p_phy_mem->size = p_mem_tbl->size;
				p_phy_mem->p_bitmap = init_bitmap + bits_init_num;

				bits_init_num += p_phy_mem->size / 4096;
				bits_init_num += (bits_init_num % 8 ? 0 : 8 - bits_init_num % 8);
				rtl_list_insert_after(&phymem_bitmap_list,
				                      NULL,
				                      p_phy_mem,
				                      phymem_heap);

			} else {
				p_phy_mem = mm_hp_alloc(sizeof(phymem_bitmap_t), phymem_heap);
				p_phy_mem->base = p_mem_tbl->base;
				p_phy_mem->size = (PHY_INIT_BITMAP_NUM - bits_init_num) * 4096;
				p_phy_mem->p_bitmap = init_bitmap + bits_init_num;

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
	//TODO:Free pages

	//Free memory
	mm_hp_free(p_this, phymem_heap);
	return;
}

pkstring_t to_string(pphymem_obj_t p_this)
{
	pkstring_t p_ret;
	pkstring_t p_memstr;
	pphymem_block_t p_block;
	int i;

	p_ret = rtl_ksprintf(NULL, "Physical memory object %P:\n%-10s%-10s\n",
	                     p_this,
	                     "Base",
	                     "Size");

	for(i = 0, p_block = p_this->blocks;
	    i < p_this->block_num;
	    i++, p_block++) {
		p_memstr = rtl_ksprintf(NULL, "%-10P%-10P\n",
		                        p_block->base, p_block->num);
		p_ret = rtl_kstrcat(p_ret, p_memstr);
		om_dec_kobject_ref(p_memstr);
	}

	return p_ret;
}
