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
#include "phymem.h"

list_t		phymem_list = NULL;
list_t		phymem_allocatable_list = NULL;

static	void	print_phymem();

void phymem_init()
{
	dbg_kprint("Testing physical memory...\n");

	#ifdef	X86
	phymem_init_x86();
	#endif	//!	X86

	print_phymem();
	return;
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
		           (u8*)p_entry->base + p_entry->size - 1,
		           p_entry->size,
		           type_str);
		p_node = p_node->p_next;
	} while(p_node != phymem_list);

	return;
}