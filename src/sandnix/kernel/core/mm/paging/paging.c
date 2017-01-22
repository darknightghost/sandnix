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

#include "paging.h"
#include "./page_obj.h"
#include "../heap/heap.h"
#include "../../pm/pm.h"
#include "../../../hal/cpu/cpu.h"
#include "../../exception/exception.h"
#include "../../../hal/mmu/mmu.h"
#include "../../rtl/rtl.h"
#include "../../kconsole/kconsole.h"

//Kernel page table
static	map_t			krnl_pg_addr_map;
static	map_t			krnl_pg_size_map;
static	map_t			krnl_pg_used_map;
static	spnlck_rw_t		krnl_pg_tbl_lck;

//User space page table
static	array_t			usr_pg_tbls;
static	spnlck_rw_t		usr_pg_tbls_lck;

//Current page tables
static	u32				current_pg_tbl[MAX_CPU_NUM];

//Heap
static	pheap_t			paging_heap = NULL;
static	u8				paging_heap_buf[SANDNIX_KERNEL_PAGE_SIZE];


static	int				compare_pageblock_addr(ppage_block_t p1, ppage_block_t p2);
static	int				compare_pageblock_size(ppage_block_t p1, ppage_block_t p2);
static	void			create_krnl();
static	void			create_0();
static	void			collect_fragment(ppage_block_t p_block,
        pmap_t p_size_map, pmap_t p_addr_map);
static	ppage_block_t	alloc_page(pmap_t p_size_map, pmap_t p_addr_map,
                                   pmap_t p_used_map);
static	void			free_page(ppage_block_t, p_block,
                                  pmap_t p_size_map,
                                  pmap_t p_addr_map,
                                  pmap_t p_used_map);

void core_mm_paging_init()
{
    core_kconsole_print_info("Initialize paging...\n");

    //Create heap
    paging_heap = core_mm_heap_create_on_buf(
                      HEAP_MULITHREAD | HEAP_PREALLOC,
                      SANDNIX_KERNEL_PAGE_SIZE,
                      paging_heap_buf,
                      sizeof(paging_heap_buf));

    if((paging_heap) == NULL) {
        PANIC(ENOMEM, "Unable to create heap for mm.\n");
    }

    core_rtl_memset(current_pg_tbl, 0, sizeof(current_pg_tbl));

    //Initialize kernel page table
    core_kconsole_print_info("Creating kernel page table...\n");
    core_rtl_map_init(&krnl_pg_addr_map, (item_compare_t)compare_pageblock_addr,
                      paging_heap);
    core_rtl_map_init(&krnl_pg_size_map, (item_compare_t)compare_pageblock_size,
                      paging_heap);
    core_rtl_map_init(&krnl_pg_used_map, (item_compare_t)compare_pageblock_addr,
                      paging_heap);
    core_pm_spnlck_rw_init(&krnl_pg_tbl_lck);
    create_krnl();


    //Initialize userspace page table
    core_kconsole_print_info("Creating page table 0...\n");
    core_rtl_array_init(&usr_pg_tbls, MAX_PROCESS_NUM, paging_heap);
    core_pm_spnlck_rw_init(&usr_pg_tbls_lck);

    //Create page table 0
    create_0();

    return;
}

void core_mm_paging_cpu_core_init(u32 cpuid)
{
    current_pg_tbl[cpuid] = 0;
    return;
}

void core_mm_paging_cpu_core_release(u32 cpuid)
{
    current_pg_tbl[cpuid] = 0;
    return;
}

void core_mm_switch_to(u32 index);
u32 core_mm_get_current_pg_tbl_index();
void core_mm_pg_tbl_fork(u32 src_index, u32 dest_index);
void core_mm_pg_tbl_clear(u32 index);
void core_mm_pg_tbl_release(u32 src_index);
void* core_mm_pg_alloc(void* base_addr, size_t size, u32 options);
void core_mm_pg_free(void* base_addr);
ppage_obj_t core_mm_get_pg_obj(void** p_base_addr, void* addr);
void* core_mm_map(void* addr, ppage_obj_t p_page_obj, u32 options);
void core_mm_commit(void* addr);
void core_mm_uncommit(void* addr);
u32 core_mm_get_pg_attr(void* address);
u32 core_mm_set_pg_attr(void* address, u32 attr);

int compare_pageblock_addr(ppage_block_t p1, ppage_block_t p2)
{
    if(p1->begin > p2->begin) {
        return 1;

    } else if(p1->begin == p2->begin) {
        return 0;

    } else {
        return -1;
    }
}

int compare_pageblock_size(ppage_block_t p1, ppage_block_t p2)
{
    if(p1->size > p2->size) {
        return 1;

    } else if(p1->size == p2->size) {
        return 0;

    } else {
        return -1;
    }
}

void create_krnl()
{
    size_t size;
    address_t base_addr = 0;
    address_t phy_begin;
    u32 attr;

    //Create empty page table
    ppage_block_t p_page_block = core_mm_heap_alloc(
                                     sizeof(page_block_t),
                                     paging_heap);

    if(p_page_block == NULL) {
        PANIC(ENOMEM, "Failed to create kernel page table.\n");
    }

    hal_mmu_get_krnl_addr_range(
        (void**)(&(p_page_block->begin)),
        &(p_page_block->size));

    p_page_block->p_prev = NULL;
    p_page_block->p_next = NULL;
    p_page_block->p_pg_obj = NULL;
    p_page_block->status = 0;

    if(core_rtl_map_set(
           &krnl_pg_addr_map,
           (void*)(p_page_block->begin),
           p_page_block) == NULL) {
        PANIC(ENOMEM, "Failed to add first kernel page block.\n");
    }

    if(core_rtl_map_set(
           &krnl_pg_size_map,
           (void*)(p_page_block->size),
           p_page_block) == NULL) {
        PANIC(ENOMEM, "Failed to add first kernel page block.\n");
    }

    //Scan mmu page table
    core_kconsole_print_debug("Memory info:\n"
                              "%-16s%-16s%-10s\n",
                              "Virtual address",
                              "Physical address",
                              "size");

    while(hal_mmu_get_next_mapped_pages((void*)base_addr,
                                        (void**)(&base_addr),
                                        (void**)(&phy_begin),
                                        &size, &attr)) {
        core_kconsole_print_debug("%-16p%-16p%-10p\n",
                                  base_addr,
                                  phy_begin,
                                  size);
        //Get page block
        p_page_block = alloc_page(
                           &krnl_pg_size_map,
                           &krnl_pg_addr_map,
                           &krnl_pg_used_map);

        if(p_page_block == NULL) {
            PANIC(ENOMEM, "Failed to get page block.\n");
        }

        //Create page object
    }
}

void create_0();
