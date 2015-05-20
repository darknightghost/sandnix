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

#include "physical_mem.h"
#include "../../../../../common/arch/x86/kernel_image.h"
#include "../../../../setup/setup.h"
#include "../../../../debug/debug.h"

#define	PHY_PAGE_INFO(addr)	phy_mem_info[(addr)/4/1024]

static	u32		phy_mem_info[1024 * 1024];

static	void	print_phy_mem();

void setup_e820()
{
	u32 num;
	u32 type;
	u32 i, base;
	pe820_table p_table;

	//Get e820 addr
	num = **(u32**)KERNEL_MEM_INFO;

	dbg_print("e820 info:\n");
	dbg_print("%-12s%-12s%-12s\n", "Base", "Size", "Type");

	for(p_table = (pe820_table)(*(u32**)KERNEL_MEM_INFO + 1), i = 0;
	    i < num;
	    i++, p_table++) {
		dbg_print("0x%X\t", p_table->base_addr_l);
		dbg_print("0x%X\t", p_table->len_l);

		switch(p_table->type) {
		case E820_USABLE:
			dbg_print("%12s\n", "E820_USABLE");
			break;

		case E820_RESERVED:
			dbg_print("%12s\n", "E820_RESERVED");
			break;

		default:
			dbg_print("%12s\n", "Unknow");
		}

	}

	//Anlyse e820
	for(base = 0; base < 1024 * 1024; base++) {
		type = 0;

		for(p_table = (pe820_table)(*(u32**)KERNEL_MEM_INFO + 1), i = 0;
		    i < num && p_table->base_addr_h == 0;
		    i++, p_table++) {
			if(base * 4 * 1024 > p_table->base_addr_l
			   && base * 4 * 1024 <= (u64)(p_table->base_addr_l) + p_table->len_l) {
				if(type < p_table->type) {
					type = p_table->type;
				}
			}
		}

		if(type == E820_USABLE) {
			if(base >= (KERNEL_BASE - VIRTUAL_ADDR_OFFSET) / 4 / 1024
			   && base <= (TMP_PAGE_TABLE_BASE + TMP_PAGE_SIZE) / 4 / 1024) {
				phy_mem_info[base] = PHY_PAGE_SYSTEM;

			} else {
				phy_mem_info[base] = PHY_PAGE_USABLE;
			}

		} else {
			if(base < 1024 * 1024 / 4 / 1024) {
				phy_mem_info[base] = PHY_PAGE_RESERVED;

			} else {
				phy_mem_info[base] = PHY_PAGE_UNUSABLE;
			}
		}

		phy_mem_info[base] = type;
	}

	print_phy_mem();

	return;
}

void print_phy_mem()
{
	u32 type;

	u32 i, base;
	dbg_print("Physical memory info:\n");
	dbg_print("%12s%12s%12s\n", "Base", "Size", "Type");

	type = phy_mem_info[0];;
	base = 0;

	for(i = 0; i < 1024 * 1024; i++) {
		if(type != phy_mem_info[i] && i != 0) {
			dbg_print("unusable ");
			//dbg_print("%12P%12P%12P\n", base, (i-1)*4*1024-base, 0);
			type = phy_mem_info[i];
		}
	}

	i--;
	dbg_print("Physical memory info end\n");
	return;
}
