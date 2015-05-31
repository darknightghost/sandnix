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
#include "../../../../rtl/rtl.h"
#include "../../../../pm/pm.h"
#include "../../../../exceptions/exceptions.h"

#define	PHY_PAGE_INFO(addr)	phy_mem_info[(addr)/4/1024]

static	phy_page_state	phy_mem_info[1024 * 1024];
static	spin_lock		mem_info_lock;

static	void			print_phy_mem();
static	void			print_e820();
static	u32				phy_mem_pg_num;
static	u32				phy_mem_pg_usable_num;

void init_phy_mem()
{
	u32 num;
	u32 i, base, end;
	pe820_table p_table;

	print_e820();

	//Anlyse e820
	dbg_print("Analyzing...\n");
	rtl_memset(phy_mem_info, 0, sizeof(phy_mem_info));
	num = **(u32**)KERNEL_MEM_INFO;

	for(p_table = (pe820_table)(*(u32**)KERNEL_MEM_INFO + 1), i = 0;
	    i < num && p_table->base_addr_h == 0;
	    i++, p_table++) {
		base = p_table->base_addr_l / 4096;
		end = (p_table->base_addr_l + p_table->len_l - 1) / 4096;

		while(base < end) {
			if(p_table->type == E820_USABLE) {
				if(phy_mem_info[base].status != PHY_PAGE_RESERVED
				   && phy_mem_info[base].status != PHY_PAGE_UNUSABLE
				   && phy_mem_info[base].status != PHY_PAGE_SYSTEM) {
					if(base >= (KERNEL_BASE - VIRTUAL_ADDR_OFFSET) / 4096
					   && base <= (TMP_PAGE_TABLE_BASE + TMP_PAGE_SIZE) / 4096) {
						//Memory of kernel image
						phy_mem_info[base].status = PHY_PAGE_SYSTEM;

					} else {
						//Usable memories
						phy_mem_info[base].status = PHY_PAGE_USABLE;
					}
				}

			} else {
				if(base < 1024 * 1024 / 4096) {
					//Reserved
					phy_mem_info[base].status = PHY_PAGE_RESERVED;

				} else {
					//Unusable
					phy_mem_info[base].status = PHY_PAGE_UNUSABLE;
				}
			}

			base++;
		}

	}

	//Set up memories which e820 didn't refered
	//and count RAM and usable memory
	phy_mem_pg_num = 0;
	phy_mem_pg_usable_num = 0;

	for(base = 0; base < 1024 * 1024; base++) {
		if(phy_mem_info[base].status == PHY_PAGE_USABLE) {
			phy_mem_pg_num++;
			phy_mem_pg_usable_num++;

		} else if(phy_mem_info[base].status == PHY_PAGE_SYSTEM) {
			phy_mem_pg_num++;

		} else if(phy_mem_info[base].status == 0) {
			if(base <= 1024 * 1024 / 4096) {
				//Reserved
				phy_mem_info[base].status = PHY_PAGE_RESERVED;

			} else {
				//Unusable
				phy_mem_info[base].status = PHY_PAGE_UNUSABLE;
			}
		}
	}

	print_phy_mem();
	pm_init_spn_lock(&mem_info_lock);

	return;
}

void* alloc_physcl_page(void* base_addr, u32 num)
{
	u32 base;
	u32 next;
	u32 i;

	if(IS_DMA_MEM(base_addr)) {
		return NULL;
	}

	if(phy_mem_pg_usable_num == 0) {
		return NULL;
	}

	pm_acqr_spn_lock(&mem_info_lock);

	if(base_addr != NULL) {
		base = (u32)base_addr / 4096;

		for(i = 0, next = base;
		    i < num;
		    i++, next++) {
			//Check if the memory is usable
			if(phy_mem_info[next].status != PHY_PAGE_USABLE) {
				pm_rls_spn_lock(&mem_info_lock);
				return NULL;
			}
		}

		for(i = 0, next = base;
		    i < num;
		    i++, next++) {

			//Allocate memory
			phy_mem_info[next].status = PHY_PAGE_ALLOCATED;
			phy_mem_info[next].ref_count = 1;
		}

		pm_rls_spn_lock(&mem_info_lock);
		phy_mem_pg_usable_num -= num;
		return base_addr;
	}

	for(base = 1024 * 1024 / 4096;
	    base < 1024 * 1024;
	    base++) {
		if(phy_mem_info[base].status == PHY_PAGE_USABLE)	{
			//Check number of usable pages
			for(i = 0, next = base;
			    i < num;
			    i++, next++) {
				//Check if the memory is usable
				if(phy_mem_info[next].status != PHY_PAGE_USABLE) {
					base += i;
					break;
				}
			}

			if(i == num) {
				//Allocate memory
				for(i = 0, next = base;
				    i < num;
				    i++, next++) {
					phy_mem_info[next].status = PHY_PAGE_ALLOCATED;
					phy_mem_info[next].ref_count = 1;
				}

				pm_rls_spn_lock(&mem_info_lock);
				phy_mem_pg_usable_num -= num;

				return (void*)(base * 4096);
			}
		}
	}

	pm_rls_spn_lock(&mem_info_lock);
	return NULL;
}

bool alloc_dma_physcl_page(void* base_addr, u32 num, void** ret)
{
	u32 base;
	u32 next;
	u32 i;

	if(!IS_DMA_MEM(base_addr)) {
		return false;
	}

	if(phy_mem_pg_usable_num == 0) {
		return false;
	}

	pm_acqr_spn_lock(&mem_info_lock);

	if(base_addr != NULL) {
		base = (u32)base_addr / 4096;

		for(i = 0, next = base;
		    i < num;
		    i++, next++) {
			//Check if the memory is usable
			if(phy_mem_info[next].status != PHY_PAGE_USABLE) {

				pm_rls_spn_lock(&mem_info_lock);
				return false;
			}
		}

		for(i = 0, next = base;
		    i < num;
		    i++, next++) {
			//Allocate memory
			phy_mem_info[next].status = PHY_PAGE_ALLOCATED;
			phy_mem_info[next].ref_count = 1;
		}

		phy_mem_pg_usable_num -= num;
		*ret = base_addr;
		return true;
	}

	for(base = 0;
	    base < 1024 * 1024 / 4096;
	    base++) {
		if(phy_mem_info[base].status == PHY_PAGE_USABLE)	{
			//Check number of usable pages
			for(i = 0, next = base;
			    i < num;
			    i++, next++) {
				//Check if the memory is usable
				if(phy_mem_info[next].status != PHY_PAGE_USABLE) {
					base += i;
					break;
				}
			}

			if(i == num) {
				//Allocate memory
				for(i = 0, next = base;
				    i < num;
				    i++, next++) {
					phy_mem_info[next].status = PHY_PAGE_ALLOCATED;
					phy_mem_info[next].ref_count = 1;
				}

				pm_rls_spn_lock(&mem_info_lock);
				phy_mem_pg_usable_num -= num;

				*ret = (void*)(base * 4096);
				return true;
			}
		}
	}

	pm_rls_spn_lock(&mem_info_lock);
	return false;
}

void increase_physcl_page_ref(void* base_addr, u32 num)
{
	u32 i;
	u32 base;

	pm_acqr_spn_lock(&mem_info_lock);
	base = (u32)base_addr / 4096;

	for(i = 0; i < num; i++) {
		if(phy_mem_info[base].status != PHY_PAGE_ALLOCATED) {
			excpt_panic(EXCEPTION_ILLEGAL_MEM_ADDR,
			            "This is because some program tried to increase the reference count of a physical page which cannot be increased.The address of the physical memory is %p.",
			            base * 4096);
		}

		(phy_mem_info[base + i].ref_count)++;
	}

	pm_rls_spn_lock(&mem_info_lock);

	return;
}

u32	get_ref_count(void* base_addr)
{
	return phy_mem_info[(u32)base_addr / 4096].ref_count;
}

void free_physcl_page(void* base_addr, u32 num)
{
	u32 i;
	u32 base;

	pm_acqr_spn_lock(&mem_info_lock);
	base = (u32)base_addr / 4096;

	for(i = 0; i < num; i++) {
		if(phy_mem_info[base].status != PHY_PAGE_ALLOCATED) {
			excpt_panic(EXCEPTION_ILLEGAL_MEM_ADDR,
			            "This is because some program tried to free a physical page which cannot be freed.The address of the physical memory is %p.",
			            base * 4096);
		}

		(phy_mem_info[base + i].ref_count)--;

		if(phy_mem_info[base + i].ref_count == 0) {
			phy_mem_info[base].status = PHY_PAGE_USABLE;
			phy_mem_pg_usable_num += num;
		}
	}

	pm_rls_spn_lock(&mem_info_lock);

	return;
}

void get_phy_mem_info(u32* phy_mem_num, u32* usable_num)
{
	*phy_mem_num = phy_mem_pg_num * 4096;
	*usable_num = phy_mem_pg_usable_num * 4096;

	return;
}


phy_page_state mm_phy_mem_state_get(void* addr)
{
	return phy_mem_info[(u32)addr / 4096];
}

void print_e820()
{
	u32 i;
	pe820_table p_table;
	u32 num;

	dbg_print("Bios e820 info:\n");
	dbg_print("%-25s%-12s\n", "Range", "Type");

	num = **(u32**)KERNEL_MEM_INFO;

	for(p_table = (pe820_table)(*(u32**)KERNEL_MEM_INFO + 1), i = 0;
	    i < num && p_table->base_addr_h == 0;
	    i++, p_table++) {
		dbg_print("%-p-->%-12p",
		          p_table->base_addr_l,
		          (u32)((u64)(p_table->len_l) + p_table->base_addr_l - 1));

		switch(p_table->type) {
		case E820_USABLE:
			dbg_print("%-12s\n", "E820_USABLE");
			break;

		case E820_RESERVED:
			dbg_print("%-12s\n", "E820_RESERVED");
			break;

		default:
			dbg_print("%-12s\n", "Unknow");
		}

	}

	dbg_print("\n");
	return;
}

void print_phy_mem()
{
	u32 type;

	u32 i, base;
	dbg_print("Physical memory info:\n");
	dbg_print("%-25s%-12s\n", "Range", "Type");

	type = phy_mem_info[0].status;
	base = 0;

	for(i = 0; i < 1024 * 1024; i++) {
		if(type != phy_mem_info[i].status && i != 0) {
			dbg_print("%-p-->%-12p", base, i * 4096 - 1);

			switch(type) {
			case 0:
				dbg_print("%-12s\n", "0");
				break;

			case PHY_PAGE_RESERVED:
				dbg_print("%-12s\n", "PHY_PAGE_RESERVED");
				break;

			case PHY_PAGE_USABLE:
				dbg_print("%-12s\n", "PHY_PAGE_USABLE");
				break;

			case PHY_PAGE_UNUSABLE:
				dbg_print("%-12s\n", "PHY_PAGE_UNUSABLE");
				break;

			case PHY_PAGE_SYSTEM:
				dbg_print("%-12s\n", "PHY_PAGE_SYSTEM");
				break;

			case PHY_PAGE_ALLOCATED:
				dbg_print("%-12s\n", "PHY_PAGE_ALLOCATED");
				break;

			default:
				dbg_print("%-12s\n", "Unknow");
			}

			type = phy_mem_info[i].status;
			base = i * 4096;
		}
	}


	dbg_print("%-p-->%-12p", base, i * 4096 - 1);

	switch(type) {
	case PHY_PAGE_RESERVED:
		dbg_print("%-12s\n", "PHY_PAGE_RESERVED");
		break;

	case PHY_PAGE_USABLE:
		dbg_print("%-12s\n", "PHY_PAGE_USABLE");
		break;

	case PHY_PAGE_UNUSABLE:
		dbg_print("%-12s\n", "PHY_PAGE_UNUSABLE");
		break;

	case PHY_PAGE_SYSTEM:
		dbg_print("%-12s\n", "PHY_PAGE_SYSTEM");
		break;

	case PHY_PAGE_ALLOCATED:
		dbg_print("%-12s\n", "PHY_PAGE_ALLOCATED");
		break;

	default:
		dbg_print("%-12s\n", "Unknow");
	}

	dbg_print("\n");
	return;
}
