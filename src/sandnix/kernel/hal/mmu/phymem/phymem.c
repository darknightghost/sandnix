/*
    Copyright 2016,王思远 <darknightghost.cn@gmail.com>

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

#include "phymem.h"
#include "../../init/init.h"
#include "../../early_print/early_print.h"

#define	IN_RANGE(n, start, size) ((n) >= (start) && (n) <= (start) + (size))

//static	map_t			phymem_info_map;
static	bool			if_conflict(plist_node_t p_pos1, plist_node_t p_pos2);
static	void			merge_mem(plist_node_t* p_p_pos1, plist_node_t* p_p_pos2);

void phymem_init()
{
    list_t info_list;
    plist_node_t p_pos1;
    plist_node_t p_pos2;
    pphysical_memory_info_t p_phy_mem;
    char* type_str;

    //Get physical memeory information from bootloader.
    info_list = hal_init_get_boot_memory_map();

    //Print boot informatiom
    hal_early_print_printf("\nPhysical memory infomation from bootloader:\n");
    hal_early_print_printf("%-20s%-20s%s\n", "Base", "Size", "Type");

    for(p_pos1 = info_list;
        p_pos1 != NULL;
        p_pos1 = core_rtl_list_next(p_pos1, &info_list)) {
        p_phy_mem = (pphysical_memory_info_t)(p_pos1->p_item);

        switch(p_phy_mem->type) {
            case PHYMEM_AVAILABLE:
                type_str = "PHYMEM_AVAILABLE";
                break;

            case PHYMEM_DMA:
                type_str = "PHYMEM_DMA";
                break;

            case PHYMEM_USED:
                type_str = "PHYMEM_USED";
                break;

            case PHYMEM_DMA_USED:
                type_str = "PHYMEM_DMA_USED";
                break;

            case PHYMEM_SYSTEM:
                type_str = "PHYMEM_SYSTEM";
                break;

            case PHYMEM_RESERVED:
                type_str = "PHYMEM_RESERVED";
                break;

            case PHYMEM_BAD:
                type_str = "PHYMEM_BAD";
                break;
        }

        hal_early_print_printf("0x%-18.16llX0x%-18.16llX%s\n", p_phy_mem->begin,
                               p_phy_mem->size, type_str);
    }

    //Merg conflict memory blocks.
    p_pos1 = info_list;

    while(core_rtl_list_next(p_pos1, &info_list) != NULL) {
        p_pos2 = core_rtl_list_next(p_pos1, &info_list);

        while(true) {
            if(if_conflict(p_pos1, p_pos2)) {
                merge_mem(&p_pos1, &p_pos2);
                continue;

            } else {
                p_pos2 = core_rtl_list_next(p_pos2, &info_list);
            }

            if(p_pos2 == NULL) {
                p_pos1 = core_rtl_list_next(p_pos1, &info_list);
                break;
            }
        }
    }

    archecture_phyaddr_edit(&info_list);
    return;
}

kstatus_t hal_mmu_phymem_alloc(
    void** p_addr,		//Start address
    bool is_dma,		//DMA page
    size_t page_num);	//Num

void hal_mmu_phymem_free(
    void* addr,			//Address
    size_t page_num);	//Num

size_t hal_mmu_get_phymem_info(
    pphysical_memory_info_t p_buf,	//Pointer to buffer
    size_t size);

bool if_conflict(plist_node_t p_pos1, plist_node_t p_pos2)
{
    pphysical_memory_info_t p1;
    pphysical_memory_info_t p2;

    p1 = (pphysical_memory_info_t)(p_pos1->p_item);
    p2 = (pphysical_memory_info_t)(p_pos2->p_item);

    if(IN_RANGE(p1->begin, p2->begin, p2->size)
       || IN_RANGE(p1->begin + p1->size - 1, p2->begin, p2->size)
       || IN_RANGE(p2->begin, p1->begin, p1->size)
       || IN_RANGE(p2->begin + p2->size - 1, p1->begin, p1->size)) {
        return true;

    } else {
        return false;
    }
}

void merge_mem(plist_node_t* p_p_pos1, plist_node_t* p_p_pos2)
{
    UNREFERRED_PARAMETER(p_p_pos1);
    UNREFERRED_PARAMETER(p_p_pos2);
}
