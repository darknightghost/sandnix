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

#define PAGE_SIZE_ALIGN(size)		((((size) / SANDNIX_KERNEL_PAGE_SIZE) \
                                      + ((size) % SANDNIX_KERNEL_PAGE_SIZE \
                                              ? 1 \
                                              : 0)) \
                                     * SANDNIX_KERNEL_PAGE_SIZE)

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
                                   pmap_t p_used_map, void* base_addr,
                                   size_t size);
static	void			free_page(ppage_block_t p_block,
                                  pmap_t p_size_map,
                                  pmap_t p_addr_map,
                                  pmap_t p_used_map);

//Search function
static	int				search_size(size_t size, ppage_block_t p_key,
                                    ppage_block_t p_value, ppage_block_t* p_p_prev);

static	int				search_addr(address_t address, ppage_block_t p_key,
                                    ppage_block_t p_value, void* p_args);



//Exception handlers
static	except_stat_t page_read_except_hndlr(
    except_reason_t reason,
    pepageread_except_t p_except);
static	except_stat_t page_write_except_hndlr(
    except_reason_t reason,
    pepagewrite_except_t p_except);
static	except_stat_t page_exec_except_hndlr(
    except_reason_t reason,
    pepageexec_except_t p_except);

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

    //Regist global page fault handlers
    core_exception_add_hndlr(EPAGEREAD,
                             (except_hndlr_t)page_read_except_hndlr);
    core_exception_add_hndlr(EPAGEWRITE,
                             (except_hndlr_t)page_write_except_hndlr);
    core_exception_add_hndlr(EPAGEEXEC,
                             (except_hndlr_t)page_exec_except_hndlr);
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
        return compare_pageblock_addr(p1, p2);

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
           p_page_block,
           p_page_block) == NULL) {
        PANIC(ENOMEM, "Failed to add first kernel page block.\n");
    }

    if(core_rtl_map_set(
           &krnl_pg_size_map,
           p_page_block,
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
                           &krnl_pg_used_map,
                           (void*)base_addr,
                           size);

        if(p_page_block == NULL) {
            PANIC(ENOMEM, "Failed to get page block.\n");
        }

        //Create page object
        p_page_block->p_pg_obj = page_obj_on_phymem(phy_begin, size,
                                 TO_BOOL(attr & MMU_PAGE_CACHEABLE));

        if(p_page_block->p_pg_obj == NULL) {
            PANIC(ENOMEM, "Failed to create page object.\n");
        }

        //Set page attribute
        p_page_block->status |= PAGE_AVAIL;

        if(attr & MMU_PAGE_WRITABLE) {
            p_page_block->status |= PAGE_WRITABLE;

        } else if(attr & MMU_PAGE_EXECUTABLE) {
            p_page_block->status |= PAGE_EXECUTABLE;
        }
    }

    return;
}

void create_0()
{
    void* usr_mem_base;
    size_t usr_mem_size;

    //Get memory range
    hal_mmu_get_usr_addr_range(&usr_mem_base, &usr_mem_size);

    //Create maps
    pproc_pg_tbl_t p_page_tbl_0 = core_mm_heap_alloc(
                                      sizeof(proc_pg_tbl_t),
                                      paging_heap);

    if(p_page_tbl_0 == NULL) {
        PANIC(ENOMEM, "Failed to create page table 0.\n");
    }

    p_page_tbl_0->id = 0;
    core_rtl_map_init(&(p_page_tbl_0->used_map),
                      (item_compare_t)compare_pageblock_addr,
                      paging_heap);
    core_rtl_map_init(&(p_page_tbl_0->free_addr_map),
                      (item_compare_t)compare_pageblock_addr,
                      paging_heap);
    core_rtl_map_init(&(p_page_tbl_0->free_size_map),
                      (item_compare_t)compare_pageblock_size,
                      paging_heap);

    //Add page block
    ppage_block_t p_page_block = core_mm_heap_alloc(
                                     sizeof(page_block_t),
                                     paging_heap);

    if(p_page_block == NULL) {
        PANIC(ENOMEM, "Failed to create page table 0.\n");
    }

    p_page_block->begin = (address_t)usr_mem_base;
    p_page_block->size = usr_mem_size;
    p_page_block->p_prev = NULL;
    p_page_block->p_next = NULL;
    p_page_block->p_pg_obj = NULL;
    p_page_block->status = 0;

    if(core_rtl_map_set(
           &(p_page_tbl_0->free_addr_map),
           p_page_block,
           p_page_block) == NULL) {
        PANIC(ENOMEM, "Failed to add first user page block.\n");
    }

    if(core_rtl_map_set(
           &(p_page_tbl_0->free_size_map),
           p_page_block,
           p_page_block) == NULL) {
        PANIC(ENOMEM, "Failed to add first user page block.\n");
    }

    //Add page table
    if(core_rtl_array_set(&usr_pg_tbls, 0, p_page_tbl_0) == NULL) {
        PANIC(ENOMEM, "Failed to add page table 0.\n");
    }

    return;
}

void collect_fragment(ppage_block_t p_block, pmap_t p_size_map,
                      pmap_t p_addr_map)
{
    //Get position to begin merge
    while(p_block->p_prev != NULL
          && !(p_block->p_prev->status & PAGE_ALLOCATED)) {
        p_block = p_block->p_prev;
    }

    //Merge blocks
    //Remove p_block from map
    core_rtl_map_set(p_size_map, p_block, NULL);
    core_rtl_map_set(p_addr_map, p_block, NULL);

    while(p_block->p_next != NULL
          && !(p_block->p_next->status & PAGE_ALLOCATED)) {
        //Remove next block from map
        ppage_block_t p_next = p_block->p_next;
        core_rtl_map_set(p_size_map, p_next, NULL);
        core_rtl_map_set(p_addr_map, p_next, NULL);

        //Merge block
        p_block->size += p_next->size;
        p_next->p_next->p_prev = p_block;
        p_block->p_next = p_next->p_next;

        //Free block
        core_mm_heap_free(p_next, paging_heap);
    }

    //Add p_block
    if(core_rtl_map_set(p_size_map, p_block, p_block) == NULL) {
        PANIC(ENOMEM, "Failed to insert memory block");
    }

    if(core_rtl_map_set(p_addr_map, p_block, p_block) == NULL) {
        PANIC(ENOMEM, "Failed to insert memory block");
    }

    return;
}

ppage_block_t alloc_page(pmap_t p_size_map, pmap_t p_addr_map,
                         pmap_t p_used_map, void* base_addr, size_t size)
{
    ppage_block_t p_page_block = NULL;

    //Get page block
    if(base_addr == NULL) {
        //Search by size
        core_rtl_map_search(
            p_size_map,
            (void*)size,
            (map_search_func_t)search_size,
            &p_page_block);

    } else {
        //Search by addr
        p_page_block = core_rtl_map_search(
                           p_addr_map,
                           (void*)base_addr,
                           (map_search_func_t)search_addr,
                           NULL);

        if(p_page_block != NULL) {
            if(p_page_block->begin + p_page_block->size
               < (address_t)base_addr + size) {
                p_page_block = NULL;
            }
        }
    }

    if(p_page_block == NULL) {
        return NULL;
    }

    //Remove page block
    core_rtl_map_set(p_size_map, p_page_block, NULL);
    core_rtl_map_set(p_addr_map, p_page_block, NULL);

    //Split page block
    if(base_addr != NULL
       && (address_t)base_addr != p_page_block->begin) {
        //Split prev block
        ppage_block_t p_prev_block = core_mm_heap_alloc(sizeof(page_block_t),
                                     paging_heap);

        if(p_prev_block == NULL) {
            goto ALLOC_PREV_BLOCK_FAILED;
        }

        p_prev_block->status = 0;
        p_prev_block->begin = p_page_block->begin;
        p_prev_block->p_pg_obj = NULL;
        p_prev_block->size = (address_t)base_addr - p_page_block->begin;
        p_prev_block->begin = p_page_block->begin;
        p_page_block->begin += size;
        p_page_block->size -= size;

        //Insert to page block list
        p_prev_block->p_prev = p_page_block->p_prev;

        if(p_page_block->p_prev != NULL) {
            p_page_block->p_prev->p_next = p_prev_block;
        }

        p_page_block->p_prev = p_prev_block;
        p_prev_block->p_next = p_page_block;

        //Add to map
        core_rtl_map_set(p_size_map, p_prev_block, p_prev_block);
        core_rtl_map_set(p_addr_map, p_prev_block, p_prev_block);
    }

    if(p_page_block->size > size) {
        //Split next page block
        ppage_block_t p_next_block = core_mm_heap_alloc(sizeof(page_block_t),
                                     paging_heap);

        if(p_next_block == NULL) {
            goto ALLOC_NEXT_BLOCK_FAILED;
        }

        p_next_block->status = 0;
        p_next_block->begin = p_page_block->begin + size;
        p_next_block->size = p_page_block->size - size;
        p_next_block->p_pg_obj = NULL;
        p_page_block->size = size;

        //Insert to page block list
        p_next_block->p_next = p_page_block->p_next;

        if(p_page_block->p_next != NULL) {
            p_page_block->p_next->p_prev = p_next_block;
        }

        p_next_block->p_prev = p_page_block;
        p_page_block->p_next = p_next_block;

        //Add to map
        core_rtl_map_set(p_size_map, p_next_block, p_next_block);
        core_rtl_map_set(p_addr_map, p_next_block, p_next_block);
    }

    //Add to map
    p_page_block->status |= PAGE_ALLOCATED;
    core_rtl_map_set(p_used_map, p_page_block, p_page_block);

    return p_page_block;

    //Exception handlers
ALLOC_NEXT_BLOCK_FAILED:
ALLOC_PREV_BLOCK_FAILED:
    core_rtl_map_set(p_size_map, p_page_block, p_page_block);
    core_rtl_map_set(p_addr_map, p_page_block, p_page_block);
    collect_fragment(p_page_block, p_size_map, p_addr_map);
    return NULL;
}

void free_page(ppage_block_t p_block, pmap_t p_size_map, pmap_t p_addr_map,
               pmap_t p_used_map)
{
    //Remove for used map
    core_rtl_map_set(p_used_map, p_block, NULL);

    //Free page object
    if(p_block->p_pg_obj != NULL) {
        DEC_REF(p_block->p_pg_obj);
        p_block->p_pg_obj = NULL;
    }

    //Insert to free map
    p_block->status = 0;
    core_rtl_map_set(p_size_map, p_block, p_block);
    core_rtl_map_set(p_addr_map, p_block, p_block);

    //Collect fragments
    collect_fragment(p_block, p_size_map, p_addr_map);

    return;
}

int	search_size(size_t size, ppage_block_t p_key, ppage_block_t p_value,
                ppage_block_t* p_p_prev)
{
    ppage_block_t p_page_block = (ppage_block_t)p_value;

    if(size > p_page_block->size) {
        *p_p_prev = p_page_block;
        return 1;

    } else if(size < p_page_block->size) {
        return -1;

    } else {
        *p_p_prev = p_page_block;
        return 0;
    }
}

int search_addr(address_t address, ppage_block_t p_key, ppage_block_t p_value,
                void* p_args)
{
    if(address < p_key->begin) {
        return -1;

    } else if(address >= p_key->begin + p_key->size) {
        return 1;

    } else {
        return 0;
    }
}

except_stat_t page_read_except_hndlr(except_reason_t reason,
                                     pepageread_except_t p_except);
except_stat_t page_write_except_hndlr(except_reason_t reason,
                                      pepagewrite_except_t p_except);
except_stat_t page_exec_except_hndlr(except_reason_t reason,
                                     pepageexec_except_t p_except);
