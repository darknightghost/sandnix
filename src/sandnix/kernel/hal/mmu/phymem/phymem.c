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
#include "../../rtl/rtl.h"
#include "../mmu.h"

#define	IN_RANGE(n, start, size) ((n) >= (start) && (n) < (start) + (size))

//static	map_t			phymem_info_map;
static	bool			initialized = false;

static	bool			should_merge(plist_node_t p_pos1, plist_node_t p_pos2);
static	void			merge_mem(plist_node_t* p_p_pos1, plist_node_t* p_p_pos2,
                                  plist_t p_list);
static	int				compare_memblock(pphysical_memory_info_t p1,
        pphysical_memory_info_t p2);

void phymem_init()
{
    list_t info_list;
    plist_node_t p_pos1;
    plist_node_t p_pos2;
    pphysical_memory_info_t p_phy_mem;
    char* type_str;
    u64 offset;
    pphysical_memory_info_t p_system_info;
    void* initrd_addr;
    size_t initrd_size;
    address_t initrd_end;

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

        //Make the memory block SANDNIX_KERNEL_PAGE_SIZE aligned.
        offset = hal_rtl_math_mod64(p_phy_mem->begin, SANDNIX_KERNEL_PAGE_SIZE);

        if(offset > 0) {
            p_phy_mem->begin -= offset;
            p_phy_mem->size += offset;
        }

        offset = hal_rtl_math_mod64(p_phy_mem->size, SANDNIX_KERNEL_PAGE_SIZE);

        if(offset > 0) {
            p_phy_mem->size += SANDNIX_KERNEL_PAGE_SIZE - offset;
        }

    }

    hal_early_print_printf("\nAnalysing...\n");
    archecture_phyaddr_edit(&info_list);

    //Kernel
    p_system_info = core_mm_heap_alloc(sizeof(physical_memory_info_t), NULL);

    p_system_info->begin = (u64)(address_t)(MIN(kernel_header.code_start,
                                            kernel_header.data_start) + get_load_pffset());
    p_system_info->size = (u64)(address_t)(MAX(kernel_header.code_start
                                           + kernel_header.code_size,
                                           kernel_header.data_start + kernel_header.data_size)
                                           + get_load_pffset() - p_system_info->begin);
    p_system_info->type = PHYMEM_SYSTEM;

    core_rtl_list_insert_after(NULL, &info_list, p_system_info, NULL);

    //Initrd
    hal_init_get_initrd_addr(&initrd_addr, &initrd_size);
    p_system_info = core_mm_heap_alloc(sizeof(physical_memory_info_t), NULL);

    p_system_info->begin = (u64)((address_t)initrd_addr / 4096 * 4096);
    initrd_end = (address_t)initrd_addr + initrd_size;

    p_system_info->size = (u64)(initrd_end % 4096 > 0
                                ? (initrd_end / 4096 + 1) * 4096
                                : initrd_end) - p_system_info->begin;
    p_system_info->type = PHYMEM_SYSTEM;

    core_rtl_list_insert_after(NULL, &info_list, p_system_info, NULL);

    //Merg conflict memory blocks.
    p_pos1 = info_list;

    while(core_rtl_list_next(p_pos1, &info_list) != NULL) {
        p_pos2 = core_rtl_list_next(p_pos1, &info_list);

        while(true) {
            if(should_merge(p_pos1, p_pos2)) {
                merge_mem(&p_pos1, &p_pos2, &info_list);
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

    core_rtl_list_qsort(&info_list, (item_compare_t)compare_memblock, false);

    //Print the result
    hal_early_print_printf("\nPhysical memory infomation:\n");
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

    //TODO: Create the heap
    //Destroy the list
    initialized = true;
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

bool should_merge(plist_node_t p_pos1, plist_node_t p_pos2)
{
    pphysical_memory_info_t p1;
    pphysical_memory_info_t p2;

    if(p_pos1 == p_pos2) {
        return false;
    }

    p1 = (pphysical_memory_info_t)(p_pos1->p_item);
    p2 = (pphysical_memory_info_t)(p_pos2->p_item);

    if((p1->begin + p1->size == p2->begin
        || p2->begin + p2->size == p1->begin)
       && p1->type == p2->type) {
        return true;
    }

    if(IN_RANGE(p1->begin, p2->begin, p2->size)
       || IN_RANGE(p1->begin + p1->size - 1, p2->begin, p2->size)
       || IN_RANGE(p2->begin, p1->begin, p1->size)
       || IN_RANGE(p2->begin + p2->size - 1, p1->begin, p1->size)) {
        return true;

    } else {
        return false;
    }
}

void merge_mem(plist_node_t* p_p_pos1, plist_node_t* p_p_pos2, plist_t p_list)
{
    plist_node_t p_pos1;
    plist_node_t p_pos2;
    plist_node_t p_ret_pos1;
    plist_node_t p_ret_pos2;
    pphysical_memory_info_t p_1;
    pphysical_memory_info_t p_2;
    pphysical_memory_info_t p_new;

    u64 begin;
    u64 end;

    p_pos1 = *p_p_pos1;
    p_pos2 = *p_p_pos2;
    p_1 = (pphysical_memory_info_t)(p_pos1->p_item);
    p_2 = (pphysical_memory_info_t)(p_pos2->p_item);

    if(p_1->type > p_2->type) {
        //1>2
        p_ret_pos1 = p_pos1;

        if(p_1->begin + p_1->size > p_2->begin
           && p_2->begin + p_2->size > p_1->begin + p_1->size
           && p_1->begin < p_2->begin) {
            //End of pos1
            end = p_2->begin + p_2->size;
            p_2->begin = p_1->begin + p_1->size;
            p_2->size = end - p_2->begin;
            p_ret_pos2 = core_rtl_list_next(p_pos2, p_list);

        } else if(p_2->begin + p_2->size > p_1->begin
                  && p_1->begin + p_1->size > p_2->begin + p_2->size
                  && p_1->begin > p_2->begin) {
            //Begining of pos1
            p_2->size = p_1->begin - p_2->begin;
            p_ret_pos2 = core_rtl_list_next(p_pos2, p_list);

        } else if(p_1->begin <= p_2->begin
                  && p_1->begin + p_1->size >= p_2->begin + p_2->size) {
            //Pos1 contains pos2
            p_ret_pos2 = core_rtl_list_prev(p_pos2, p_list);
            core_rtl_list_remove(p_pos2, p_list, NULL);

            //Merge two memory blocks
            end = MAX(p_1->begin + p_1->size, p_2->begin + p_2->size);
            begin = MIN(p_1->begin, p_2->begin);
            p_1->begin = begin;
            p_1->size = end - begin;
            core_mm_heap_free(p_2, NULL);
            p_ret_pos2 = core_rtl_list_next(p_ret_pos2, p_list);

        } else {
            //Pos2 contains pos1
            p_ret_pos2 = core_rtl_list_prev(p_pos2, p_list);
            core_rtl_list_remove(p_pos2, p_list, NULL);

            p_new = core_mm_heap_alloc(sizeof(physical_memory_info_t), NULL);
            p_new->type = p_2->type;

            p_new->begin = p_1->begin + p_1->size;
            p_new->size = p_2->begin + p_2->size - p_new->begin;
            p_2->size = p_1->begin - p_2->begin;

            if(p_2->size == 0) {
                core_mm_heap_free(p_2, NULL);

            } else {
                core_rtl_list_insert_after(NULL, p_list, p_2, NULL);
            }

            if(p_new->size == 0) {
                core_mm_heap_free(p_new, NULL);

            } else {
                core_rtl_list_insert_after(NULL, p_list, p_new, NULL);
            }

            p_ret_pos2 = core_rtl_list_next(p_ret_pos2, p_list);
        }

    } else if(p_1->type == p_2->type) {
        p_ret_pos1 = p_pos1;

        //Remove p_pos2
        core_rtl_list_remove(p_pos2, p_list, NULL);

        //Merge two memory blocks
        end = MAX(p_1->begin + p_1->size, p_2->begin + p_2->size);
        begin = MIN(p_1->begin, p_2->begin);
        p_1->begin = begin;
        p_1->size = end - begin;
        core_mm_heap_free(p_2, NULL);
        p_ret_pos2 = core_rtl_list_next(p_pos1, p_list);

    } else {
        //2>1
        if(p_2->begin + p_2->size > p_1->begin
           && p_1->begin + p_1->size > p_2->begin + p_2->size
           && p_2->begin < p_1->begin) {
            //End of pos2
            end = p_1->begin + p_1->size;
            p_1->begin = p_2->begin + p_2->size;
            p_1->size = end - p_2->begin;
            p_ret_pos1 = p_pos1;
            p_ret_pos2 = core_rtl_list_next(p_pos2, p_list);

        } else if(p_1->begin + p_1->size > p_2->begin
                  && p_2->begin + p_2->size > p_1->begin + p_1->size
                  && p_2->begin > p_1->begin) {
            //Begining of pos2
            p_1->size = p_2->begin - p_1->begin;
            p_ret_pos1 = p_pos1;
            p_ret_pos2 = core_rtl_list_next(p_pos2, p_list);

        } else if(p_2->begin <= p_1->begin
                  && p_2->begin + p_2->size >= p_1->begin + p_1->size) {
            //Pos2 contains pos1
            p_ret_pos1 = core_rtl_list_prev(p_pos1, p_list);
            core_rtl_list_remove(p_pos1, p_list, NULL);

            //Merge two memory blocks
            end = MAX(p_1->begin + p_1->size, p_2->begin + p_2->size);
            begin = MIN(p_1->begin, p_2->begin);
            p_2->begin = begin;
            p_2->size = end - begin;
            core_mm_heap_free(p_1, NULL);

            if(p_ret_pos1 == NULL) {
                p_ret_pos1 = *p_list;

            } else {
                p_ret_pos1 = core_rtl_list_next(p_ret_pos1, p_list);
            }

            p_ret_pos2 = core_rtl_list_next(p_ret_pos1, p_list);

        } else {
            //Pos1 contains pos2
            p_ret_pos1 = core_rtl_list_prev(p_pos1, p_list);
            core_rtl_list_remove(p_pos1, p_list, NULL);

            p_new = core_mm_heap_alloc(sizeof(physical_memory_info_t), NULL);
            p_new->type = p_1->type;

            p_new->begin = p_2->begin + p_2->size;
            p_new->size = p_1->begin + p_1->size - p_new->begin;
            p_1->size = p_2->begin - p_1->begin;

            if(p_1->size == 0) {
                core_mm_heap_free(p_1, NULL);

            } else {
                core_rtl_list_insert_after(NULL, p_list, p_1, NULL);
            }

            if(p_new->size == 0) {
                core_mm_heap_free(p_new, NULL);

            } else {
                core_rtl_list_insert_after(NULL, p_list, p_new, NULL);
            }

            if(p_ret_pos1 == NULL) {
                p_ret_pos1 = *p_list;

            } else {
                p_ret_pos1 = core_rtl_list_next(p_ret_pos1, p_list);
            }

            p_ret_pos2 = core_rtl_list_next(p_ret_pos1, p_list);
        }

    }

    if(p_ret_pos1 == NULL) {
        *p_p_pos1 = *p_list;

    } else {
        *p_p_pos1 = p_ret_pos1;
    }

    if(p_ret_pos2 == NULL) {
        *p_p_pos2 = *p_list;

    } else {
        *p_p_pos2 = p_ret_pos2;
    }

    return;
}

int compare_memblock(pphysical_memory_info_t p1, pphysical_memory_info_t p2)
{
    if(p1->begin > p2->begin) {
        return 1;

    } else if(p1->begin == p2->begin) {
        return 0;

    } else {
        return -1;
    }
}
