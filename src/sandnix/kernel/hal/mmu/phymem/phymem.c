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
#include "../../exception/exception.h"
#include "../mmu.h"

#define	IN_RANGE(n, start, size) ((n) >= (start) && (n) < (start) + (size))

static	map_t			free_map;
static	map_t			index_map;
#ifdef	RESERVE_DMA
    static	map_t			dma_map;
#endif
static	bool			initialized = false;
static	pheap_t			phymem_heap;
static	u8				phymem_heap_block[4096 * 2];

static	spnlck_rw_t		lock;

static	bool			should_merge(plist_node_t p_pos1, plist_node_t p_pos2);
static	void			merge_mem(plist_node_t* p_p_pos1, plist_node_t* p_p_pos2,
                                  plist_t p_list);
static	int				compare_memblock(pphysical_memory_info_t p1,
        pphysical_memory_info_t p2);
static	int				compare_addr(pphysical_memory_info_t p1,
                                     pphysical_memory_info_t p2);
static	int				compare_memblock_size(pphysical_memory_info_t p1,
        pphysical_memory_info_t p2);
static	void			print_mem_list(list_t list);
static	void			init_map(list_t mem_list);
static	int				search_addr_func(address_t base_address,
        pphysical_memory_info_t p_key,
        pphysical_memory_info_t p_value,
        void* p_arg);
static	int				search_size_func(size_t size,
        pphysical_memory_info_t p_key,
        pphysical_memory_info_t p_value,
        size_t align);
static	void			collect_fregments(pphysical_memory_info_t p_memblock,
        pmap_t p_free_map);

void phymem_init()
{
    list_t info_list;
    plist_node_t p_pos1;
    plist_node_t p_pos2;
    pphysical_memory_info_t p_phy_mem;
    u64 offset;
    pphysical_memory_info_t p_system_info;
    void* initrd_addr;
    size_t initrd_size;
    address_t initrd_end;

    core_pm_spnlck_rw_init(&lock);
    //Get physical memeory information from bootloader.
    info_list = hal_init_get_boot_memory_map();

    //Print boot informatiom
    hal_early_print_printf("\nPhysical memory infomation from bootloader:\n");
    print_mem_list(info_list);

    for(p_pos1 = info_list;
        p_pos1 != NULL;
        p_pos1 = core_rtl_list_next(p_pos1, &info_list)) {
        p_phy_mem = (pphysical_memory_info_t)(p_pos1->p_item);
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

    if(p_system_info == NULL) {
        PANIC(ENOMEM,
              "Failed to allocate memory for physical memory information.");
    }

    p_system_info->begin = (u64)(address_t)(MIN(kernel_header.code_start,
                                            kernel_header.data_start) + get_load_offset());
    p_system_info->size = (u64)(address_t)(MAX(kernel_header.code_start
                                           + kernel_header.code_size,
                                           kernel_header.data_start + kernel_header.data_size)
                                           + get_load_offset() - p_system_info->begin);
    p_system_info->type = PHYMEM_SYSTEM;

    core_rtl_list_insert_after(NULL, &info_list, p_system_info, NULL);

    //Initrd
    hal_init_get_initrd_addr(&initrd_addr, &initrd_size);
    p_system_info = core_mm_heap_alloc(sizeof(physical_memory_info_t), NULL);

    if(p_system_info == NULL) {
        PANIC(ENOMEM,
              "Failed to allocate memory for physical memory information.");
    }

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
    print_mem_list(info_list);

    //Create the heap
    phymem_heap = core_mm_heap_create_on_buf(
                      HEAP_MULITHREAD | HEAP_PREALLOC,
                      4096, phymem_heap_block, sizeof(phymem_heap_block));

    if(phymem_heap == NULL) {
        PANIC(ENOMEM,
              "Not enough memory for physical memory managment.");
    }

    //Create maps
    init_map(info_list);

    //Destroy the list
    core_rtl_list_destroy(&info_list, NULL, (item_destroyer_t)core_mm_heap_free,
                          NULL);
    initialized = true;
    return;
}

kstatus_t hal_mmu_phymem_alloc(void** p_addr, address_t align, bool is_dma, size_t page_num)
{
    if(!initialized) {
        PANIC(ENOTSUP,
              "Module has not been initialized.\n");
    }

    kstatus_t status = ESUCCESS;

    //Which map does the memblock in
    pmap_t p_free_map = &free_map;
    #ifdef	RESERVE_DMA

    if(is_dma) {
        p_free_map = &dma_map;
    }

    #endif

    core_pm_spnlck_rw_w_lock(&lock);

    //Search for memory block
    pphysical_memory_info_t p_memblock = core_rtl_map_search(p_free_map,
                                         (void*)(page_num * SANDNIX_KERNEL_PAGE_SIZE),
                                         (map_search_func_t)search_size_func,
                                         (void*)align);

    if(p_memblock == NULL) {
        status = ENOMEM;
        goto _RELEASE_SEARCH;
    }

    //Allocate memory
    size_t prev_size = hal_rtl_math_mod64(p_memblock->begin, align);

    if(prev_size != 0) {
        prev_size = align -= prev_size;
    }

    if(prev_size != 0) {
        //Split memory_block
        //Allocate memory
        pphysical_memory_info_t p_new_memblock = core_mm_heap_alloc(
                    sizeof(physical_memory_info_t), phymem_heap);

        if(p_new_memblock == NULL) {
            status = ENOMEM;
            goto _RELEASE_SEARCH;
        }

        p_new_memblock->begin = p_memblock->begin;
        p_new_memblock->size = prev_size;
        p_new_memblock->type = p_memblock->type;
        p_memblock->begin += prev_size;
        p_memblock->size -= prev_size;

        //Insert new block
        if(core_rtl_map_set(p_free_map, p_new_memblock, p_new_memblock) == NULL) {
            core_mm_heap_free(p_new_memblock, phymem_heap);
            collect_fregments(p_memblock, p_free_map);
            status = ENOMEM;
            goto _RELEASE_SEARCH;
        }

        if(core_rtl_map_set(&index_map, p_new_memblock, p_new_memblock) == NULL) {
            core_rtl_map_set(p_free_map, p_new_memblock, NULL);
            core_mm_heap_free(p_new_memblock, phymem_heap);
            collect_fregments(p_memblock, p_free_map);
            status = ENOMEM;
            goto _RELEASE_SEARCH;
        }


    }

    if(p_memblock->size > page_num * SANDNIX_KERNEL_PAGE_SIZE) {
        //Split memory block
        //Allocate memory
        pphysical_memory_info_t p_new_memblock = core_mm_heap_alloc(
                    sizeof(physical_memory_info_t), phymem_heap);

        if(p_new_memblock == NULL) {
            collect_fregments(p_memblock, p_free_map);
            status = ENOMEM;
            goto _RELEASE_SEARCH;
        }

        //Split
        p_new_memblock->begin = p_memblock->begin
                                + page_num * SANDNIX_KERNEL_PAGE_SIZE;
        p_new_memblock->size = p_memblock->size
                               - page_num * SANDNIX_KERNEL_PAGE_SIZE;
        p_new_memblock->type = p_memblock->type;

        //Insert new block
        if(core_rtl_map_set(p_free_map, p_new_memblock, p_new_memblock) == NULL) {
            core_mm_heap_free(p_new_memblock, phymem_heap);
            status = ENOMEM;
            goto _RELEASE_SEARCH;
        }

        if(core_rtl_map_set(&index_map, p_new_memblock, p_new_memblock) == NULL) {
            core_rtl_map_set(p_free_map, p_new_memblock, NULL);
            core_mm_heap_free(p_new_memblock, phymem_heap);
            status = ENOMEM;
            goto _RELEASE_SEARCH;
        }

        p_memblock->size = page_num * SANDNIX_KERNEL_PAGE_SIZE;

    }

    if(p_memblock->type == PHYMEM_AVAILABLE) {
        p_memblock->type = PHYMEM_USED;

        #ifdef	RESERVE_DMA

    } else if(p_memblock->type == PHYMEM_DMA) {
        p_memblock->type = PHYMEM_DMA_USED;
        #endif

    } else {
        PANIC(EOVERFLOW,
              "Physical memory map has been corrupted.");
    }

    core_rtl_map_set(p_free_map, p_memblock, NULL);
    *p_addr = (void*)(address_t)p_memblock->begin;

_RELEASE_SEARCH:
    core_pm_spnlck_rw_w_unlock(&lock);

    #ifndef	RESERVE_DMA
    UNREFERRED_PARAMETER(is_dma);
    #endif

    return status;
}

void hal_mmu_phymem_free(void* addr)
{
    if(!initialized) {
        PANIC(ENOTSUP,
              "Module has not been initialized.\n");
    }

    core_pm_spnlck_rw_w_lock(&lock);

    //Search for memory block
    pphysical_memory_info_t p_memblock = core_rtl_map_search(&index_map,
                                         addr,
                                         (map_search_func_t)search_addr_func,
                                         NULL);

    if(p_memblock->begin != (u64)(address_t)addr
   #if defined	RESERVE_DMA
       || (p_memblock->type != PHYMEM_USED
           && p_memblock->type != PHYMEM_DMA_USED)
   #else
       || p_memblock->type != PHYMEM_USED
   #endif
      ) {
        PANIC(EINVAL, "Illegal physical address %p for free.",
              addr);
    }

    pmap_t p_free_map;

    if(p_memblock->type == PHYMEM_USED) {
        p_memblock->type = PHYMEM_AVAILABLE;
        p_free_map = &free_map;
        #ifdef	RESERVE_DMA

    } else if(p_memblock->type == PHYMEM_DMA_USED) {
        p_memblock->type = PHYMEM_DMA;
        p_free_map = &dma_map;
        #endif
    }

    if(core_rtl_map_set(&free_map, p_memblock, p_memblock) == NULL) {
        PANIC(ENOMEM,
              "Failed to insert free memory map into physical memory map.");
    }

    //Collect fragments
    collect_fregments(p_memblock, p_free_map);

    core_pm_spnlck_rw_w_unlock(&lock);

    return;
}

size_t hal_mmu_get_phymem_info(pphysical_memory_info_t p_buf, size_t size)
{
    if(!initialized) {
        PANIC(ENOTSUP,
              "Module has not been initialized.\n");
    }

    size_t len;

    core_pm_spnlck_rw_r_lock(&lock);
    pphysical_memory_info_t p_memblock = core_rtl_map_next(&index_map, NULL);

    for(len = 0; p_memblock != NULL; len += sizeof(physical_memory_info_t)) {
        if(len < size) {
            core_rtl_memcpy(p_buf, p_memblock, sizeof(physical_memory_info_t));
        }

        p_buf++;
        p_memblock = core_rtl_map_next(&index_map, p_memblock);
    }

    core_pm_spnlck_rw_r_unlock(&lock);
    return len;
}

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

            if(p_new == NULL) {
                PANIC(ENOMEM,
                      "Failed to allocate memory for physical memory information.");
            }

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

            if(p_new == NULL) {
                PANIC(ENOMEM,
                      "Failed to allocate memory for physical memory information.");
            }

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

int compare_memblock_size(pphysical_memory_info_t p1, pphysical_memory_info_t p2)
{
    if(p1->size > p2->size) {
        return 1;

    } else if(p1->size == p2->size) {
        return compare_addr(p1, p2);

    } else {
        return -1;
    }
}

void print_mem_list(list_t list)
{
    char* type_str;
    plist_node_t p_pos;
    pphysical_memory_info_t p_phy_mem;

    hal_early_print_printf("%-20s%-20s%s\n", "Base", "Size", "Type");

    for(p_pos = list;
        p_pos != NULL;
        p_pos = core_rtl_list_next(p_pos, &list)) {
        p_phy_mem = (pphysical_memory_info_t)(p_pos->p_item);

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

    return;
}

void init_map(list_t mem_list)
{
    plist_node_t p_node;
    pphysical_memory_info_t p_phymem;
    pphysical_memory_info_t p_new_phymem;

    //Initialize maps
    core_rtl_map_init(&free_map, (item_compare_t)compare_memblock_size,
                      phymem_heap);
    core_rtl_map_init(&index_map, (item_compare_t)compare_addr,
                      phymem_heap);
    #ifdef	RESERVE_DMA
    core_rtl_map_init(&dma_map, (item_compare_t)compare_memblock_size,
                      phymem_heap);
    #endif

    //Add memory blocks to the map
    p_node = mem_list;

    do {
        p_phymem = (pphysical_memory_info_t)(p_node->p_item);
        p_new_phymem = core_mm_heap_alloc(sizeof(physical_memory_info_t),
                                          phymem_heap);

        if(p_new_phymem == NULL) {
            PANIC(ENOMEM,
                  "Failed to allocate memory for physical memory "
                  "managment module.");
        }

        core_rtl_memcpy(p_new_phymem, p_phymem, sizeof(physical_memory_info_t));

        switch(p_new_phymem->type) {
            case PHYMEM_AVAILABLE:
                if(core_rtl_map_set(&free_map, p_new_phymem, p_new_phymem)
                   == NULL) {
                    PANIC(ENOMEM,
                          "Failed to allocate memory for physical memory "
                          "managment module.");
                }

                break;

                #ifdef	RESERVE_DMA

            case PHYMEM_DMA:
                if(core_rtl_map_set(&dma_map, p_new_phymem, p_new_phymem)
                   == NULL) {
                    PANIC(ENOMEM,
                          "Failed to allocate memory for physical memory "
                          "managment module.");
                }

                break;
                #endif

            case PHYMEM_USED:
                #ifdef	RESERVE_DMA
            case PHYMEM_DMA_USED:
                #endif
                break;

            case PHYMEM_SYSTEM:
            case PHYMEM_RESERVED:
            case PHYMEM_BAD:
                break;

            default:
                PANIC(EINVAL,
                      "Illegal physical memory type :\"%u\"."
                      , p_new_phymem->type);

        }

        if(core_rtl_map_set(&index_map, p_new_phymem, p_new_phymem)
           == NULL) {
            PANIC(ENOMEM,
                  "Failed to allocate memory for physical memory "
                  "managment module.");
        }

        p_node = core_rtl_list_next(p_node, &mem_list);
    } while(p_node != NULL);

    return;
}

int search_addr_func(address_t base_address, pphysical_memory_info_t p_key,
                     pphysical_memory_info_t p_value, void* p_arg)
{
    if(IN_RANGE(base_address, p_key->begin, p_key->size)) {
        return 0;

    } else if(base_address < p_key->begin) {
        return - 1;

    } else {
        return 1;
    }

    UNREFERRED_PARAMETER(p_value);
    UNREFERRED_PARAMETER(p_arg);
}

int search_size_func(size_t size, pphysical_memory_info_t p_key,
                     pphysical_memory_info_t p_value, address_t align)
{
    size += (size_t)hal_rtl_math_mod64(p_key->begin, align);

    if(size > p_key->size) {
        return 1;

    }  else {
        return 0;
    }

    UNREFERRED_PARAMETER(p_value);
}

int compare_addr(pphysical_memory_info_t p1, pphysical_memory_info_t p2)
{
    if(p1->begin > p2->begin) {
        return 1;

    } else if(p1->begin == p2->begin) {
        return 0;

    } else {
        return -1;
    }
}

void collect_fregments(pphysical_memory_info_t p_memblock, pmap_t p_free_map)
{
    //Prev block
    pphysical_memory_info_t p_prev_blk
        = (pphysical_memory_info_t)core_rtl_map_prev(&index_map,
                p_memblock);

    if(p_prev_blk != NULL
       && p_prev_blk->type == p_memblock->type
       && p_prev_blk->begin + p_prev_blk->size == p_memblock->begin) {
        p_prev_blk->size += p_memblock->size;
        core_rtl_map_set(&index_map, p_memblock, NULL);
        core_rtl_map_set(p_free_map, p_memblock, NULL);
        core_mm_heap_free(p_memblock, phymem_heap);
        p_memblock = p_prev_blk;
    }

    //Next block
    pphysical_memory_info_t p_next_blk
        = (pphysical_memory_info_t)core_rtl_map_next(&index_map,
                p_memblock);

    if(p_next_blk != NULL
       && p_next_blk->type == p_memblock->type
       && p_memblock->begin + p_memblock->size == p_next_blk->begin) {
        p_memblock->size += p_next_blk->size;
        core_rtl_map_set(&index_map, p_next_blk, NULL);
        core_rtl_map_set(p_free_map, p_next_blk, NULL);
        core_mm_heap_free(p_next_blk, phymem_heap);
    }

}
