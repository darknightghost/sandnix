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
#include "../../rtl/rtl.h"
#include "../../pm/pm.h"
#include "../../exception/exception.h"
#include "../../kconsole/kconsole.h"

#include "../../../hal/cpu/cpu.h"
#include "../../../hal/mmu/mmu.h"

#include "./page_obj.h"
#include "../heap/heap.h"

#define PAGE_SIZE_ALIGN(size)		((((size) / SANDNIX_KERNEL_PAGE_SIZE) \
                                      + ((size) % SANDNIX_KERNEL_PAGE_SIZE \
                                              ? 1 \
                                              : 0)) \
                                     * SANDNIX_KERNEL_PAGE_SIZE)

//Kernel page table
static	map_t			krnl_pg_addr_map;
static	map_t			krnl_pg_size_map;
static	map_t			krnl_pg_used_map;
static	spnlck_t		krnl_pg_tbl_lck;

//User space page table
static	array_t			usr_pg_tbls;
static	spnlck_t		usr_pg_tbls_lck;

//Current page tables
static	u32				current_pg_tbl[MAX_CPU_NUM];

//Heap
static	pheap_t			paging_heap = NULL;
static	u8				paging_heap_buf[SANDNIX_KERNEL_PAGE_SIZE * 2];


static	int				compare_pageblock_addr(ppage_block_t p1, ppage_block_t p2);
static	int				compare_pageblock_size(ppage_block_t p1, ppage_block_t p2);
static	void			create_krnl();
static	void			create_0();
static	void			collect_fragment(ppage_block_t p_block,
        pmap_t p_size_map, pmap_t p_addr_map);
static	ppage_block_t	alloc_page(pmap_t p_size_map, pmap_t p_addr_map,
                                   pmap_t p_used_map, void* base_addr,
                                   size_t size);
static	bool			commit_page(ppage_block_t p_page_block, u32 options);
static	void			free_page(ppage_block_t p_block,
                                  pmap_t p_size_map,
                                  pmap_t p_addr_map,
                                  pmap_t p_used_map);
static	void			clear_usr_pg_tbl(pproc_pg_tbl_t p_usr_tbl);

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
static	except_stat_t deadlock_except_hndlr(
    except_reason_t reason,
    pedeadlock_except_t p_except);

void core_mm_paging_init()
{
    core_kconsole_print_info("Initialize paging...\n");

    //Create heap
    paging_heap = core_mm_heap_create_on_buf(
                      HEAP_MULITHREAD | HEAP_PREALLOC,
                      sizeof(paging_heap_buf),
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
    core_pm_spnlck_init(&krnl_pg_tbl_lck);
    create_krnl();

    //Initialize userspace page table
    core_kconsole_print_info("Creating page table 0...\n");
    core_rtl_array_init(&usr_pg_tbls, MAX_PROCESS_NUM, paging_heap);
    core_pm_spnlck_init(&usr_pg_tbls_lck);

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

void core_mm_paging_cpu_core_init(u32 cpu_index)
{
    current_pg_tbl[cpu_index] = 0;
    return;
}

void core_mm_paging_cpu_core_release(u32 cpu_index)
{
    current_pg_tbl[cpu_index] = 0;
    return;
}

void core_mm_switch_to(u32 index)
{
    u32 cpu_index = hal_cpu_get_cpu_index();

    if(current_pg_tbl[cpu_index] != index) {
        //Switch page table
        current_pg_tbl[cpu_index] = index;
        hal_mmu_pg_tbl_switch(index);
    }

    return;
}

u32 core_mm_get_current_pg_tbl_index()
{
    u32 cpu_index = hal_cpu_get_cpu_index();

    return current_pg_tbl[cpu_index];
}

void core_mm_pg_tbl_fork(u32 src_index, u32 dest_index)
{
    core_pm_spnlck_lock(&usr_pg_tbls_lck);

    //Check if the page table has been used
    if(core_rtl_array_get(&usr_pg_tbls, dest_index) != NULL) {
        core_pm_spnlck_unlock(&usr_pg_tbls_lck);
        peinval_except_t p_except = einval_except();
        RAISE(p_except, "Destination page table has been used.");
        return;
    }

    //Get source page table
    pproc_pg_tbl_t p_src_table = (pproc_pg_tbl_t)core_rtl_array_get(
                                     &usr_pg_tbls,
                                     src_index);

    if(p_src_table == NULL) {
        core_pm_spnlck_unlock(&usr_pg_tbls_lck);
        peinval_except_t p_except = einval_except();
        RAISE(p_except, "Source page table does not exists.");
        return;
    }

    //Create mmu page table
    hal_mmu_pg_tbl_create(dest_index);

    //Create dest page table
    pproc_pg_tbl_t p_dest_table = core_mm_heap_alloc(
                                      sizeof(proc_pg_tbl_t),
                                      paging_heap);

    if(core_rtl_array_set(
           &usr_pg_tbls,
           dest_index,
           p_dest_table) == NULL) {
        hal_mmu_pg_tbl_destroy(dest_index);
        core_mm_heap_free(p_dest_table, paging_heap);
        core_pm_spnlck_unlock(&usr_pg_tbls_lck);
        penomem_except_t p_except = enomem_except();
        RAISE(p_except, "Faile to add new page table.");
        return;
    }

    //Initialize new page table
    p_dest_table->id = dest_index;
    core_rtl_map_init(&(p_dest_table->free_addr_map),
                      (item_compare_t)compare_pageblock_addr,
                      paging_heap);
    core_rtl_map_init(&(p_dest_table->free_size_map),
                      (item_compare_t)compare_pageblock_size,
                      paging_heap);
    core_rtl_map_init(&(p_dest_table->used_map),
                      (item_compare_t)compare_pageblock_addr,
                      paging_heap);

    //Fork page blocks
    //Fork free map
    for(ppage_block_t p_page_block = core_rtl_map_next(
                                         &(p_src_table->free_addr_map),
                                         NULL);
        p_page_block != NULL;
        p_page_block = core_rtl_map_next(
                           &(p_src_table->free_addr_map),
                           p_page_block)) {
        //Fork page block
        ppage_block_t p_new_page_block = core_mm_heap_alloc(
                                             sizeof(page_block_t),
                                             paging_heap);

        if(p_new_page_block == NULL) {
            PANIC(ENOMEM, "Failed to fork page block.");
        }

        core_rtl_memcpy(&p_new_page_block, &p_page_block, sizeof(page_block_t));


        if(core_rtl_map_set(&(p_dest_table->free_addr_map),
                            p_new_page_block,
                            p_new_page_block) == NULL) {
            core_pm_spnlck_unlock(&usr_pg_tbls_lck);
            PANIC(ENOMEM, "Failed to fork page block.");
        }

        if(core_rtl_map_set(&(p_dest_table->free_size_map),
                            p_new_page_block,
                            p_new_page_block) == NULL) {
            core_pm_spnlck_unlock(&usr_pg_tbls_lck);
            PANIC(ENOMEM, "Failed to fork page block.");
        }
    }

    //Fork used map
    for(ppage_block_t p_page_block = core_rtl_map_next(
                                         &(p_src_table->used_map),
                                         NULL);
        p_page_block != NULL;
        p_page_block = core_rtl_map_next(
                           &(p_src_table->used_map),
                           p_page_block)) {
        //Fork page block
        ppage_block_t p_new_page_block = core_mm_heap_alloc(
                                             sizeof(page_block_t),
                                             paging_heap);

        if(p_new_page_block == NULL) {
            core_pm_spnlck_unlock(&usr_pg_tbls_lck);
            PANIC(ENOMEM, "Failed to fork page block.");
        }

        core_rtl_memcpy(&p_new_page_block, &p_page_block, sizeof(page_block_t));

        if(p_new_page_block->status & PAGE_BLOCK_COMMITED) {
            if(p_new_page_block->status & PAGE_BLOCK_SHARED) {
                //Increase reference
                INC_REF(p_new_page_block->p_pg_obj);

            } else {
                //Fork page object
                p_new_page_block->p_pg_obj =
                    p_new_page_block->p_pg_obj->fork(p_new_page_block->p_pg_obj);
            }
        }

        if(core_rtl_map_set(&(p_dest_table->used_map),
                            p_new_page_block,
                            p_new_page_block) == NULL) {
            core_pm_spnlck_unlock(&usr_pg_tbls_lck);
            PANIC(ENOMEM, "Failed to fork page block.");
        }
    }

    core_pm_spnlck_unlock(&usr_pg_tbls_lck);
    return;
}

void core_mm_pg_tbl_clear(u32 index)
{
    core_pm_spnlck_lock(&usr_pg_tbls_lck);

    //Get page table
    pproc_pg_tbl_t p_pg_tbl = (pproc_pg_tbl_t)core_rtl_array_get(
                                  &usr_pg_tbls,
                                  index);

    if(p_pg_tbl == NULL) {
        core_pm_spnlck_unlock(&usr_pg_tbls_lck);
        peinval_except_t p_except = einval_except();
        RAISE(p_except, "Page table does not exists.");
        return;
    }

    //Clear page table
    clear_usr_pg_tbl(p_pg_tbl);

    core_pm_spnlck_unlock(&usr_pg_tbls_lck);
    return;
}

void core_mm_pg_tbl_release(u32 index)
{
    core_pm_spnlck_lock(&usr_pg_tbls_lck);

    //Get page table
    pproc_pg_tbl_t p_pg_tbl = (pproc_pg_tbl_t)core_rtl_array_get(
                                  &usr_pg_tbls,
                                  index);

    if(p_pg_tbl == NULL) {
        core_pm_spnlck_unlock(&usr_pg_tbls_lck);
        peinval_except_t p_except = einval_except();
        RAISE(p_except, "Page table does not exists.");
        return;
    }

    //Clear page table
    clear_usr_pg_tbl(p_pg_tbl);

    //Free memory
    ppage_block_t p_free_block;

    while((p_free_block = (ppage_block_t)core_rtl_map_next(
                              &(p_pg_tbl->free_size_map),
                              NULL)) != NULL) {
        core_rtl_map_set(&(p_pg_tbl->free_size_map), p_free_block, NULL);
        core_rtl_map_set(&(p_pg_tbl->free_addr_map), p_free_block, NULL);
        core_mm_heap_free(p_free_block, paging_heap);
    }

    core_mm_heap_free(p_pg_tbl, paging_heap);
    core_rtl_array_set(&usr_pg_tbls, index, NULL);

    core_pm_spnlck_unlock(&usr_pg_tbls_lck);
    return;
}

void* core_mm_pg_alloc(void* base_addr, size_t size, u32 options)
{
    address_t kernel_mem_base;
    size_t kernel_mem_size;
    address_t usr_mem_base;
    address_t usr_mem_size;

    size = PAGE_SIZE_ALIGN(size +
                           ((address_t)base_addr - (address_t)base_addr
                            / SANDNIX_KERNEL_PAGE_SIZE * SANDNIX_KERNEL_PAGE_SIZE));
    base_addr = (void*)((address_t)base_addr
                        / SANDNIX_KERNEL_PAGE_SIZE * SANDNIX_KERNEL_PAGE_SIZE);

    if(base_addr != NULL) {
        //Check address
        hal_mmu_get_krnl_addr_range(
            (void**)(&kernel_mem_base),
            &kernel_mem_size);
        hal_mmu_get_usr_addr_range(
            (void**)(&usr_mem_base),
            &usr_mem_size);

        if(options & PAGE_OPTION_KERNEL) {
            if((address_t)base_addr < kernel_mem_base
               || (address_t)base_addr + size - kernel_mem_base
               >= kernel_mem_size) {
                goto EXCEPT_CHECK_ADDR;
            }

        } else {
            if((address_t)base_addr < usr_mem_base
               || (address_t)base_addr + size - usr_mem_base
               >= usr_mem_size) {
                goto EXCEPT_CHECK_ADDR;
            }
        }
    }

    //Get map
    pmap_t p_addr_map;
    pmap_t p_used_map;
    pmap_t p_size_map;
    pspnlck_t p_lock;

    if(options & PAGE_OPTION_KERNEL) {
        //Kernel memory
        p_lock = &krnl_pg_tbl_lck;
        p_addr_map = &krnl_pg_addr_map;
        p_size_map = &krnl_pg_size_map;
        p_used_map = &krnl_pg_used_map;

        core_pm_spnlck_lock(p_lock);

    } else {
        //User memory
        p_lock = &usr_pg_tbls_lck;
        u32 current_proc = core_pm_get_crrnt_proc_id();
        core_pm_spnlck_lock(p_lock);
        pproc_pg_tbl_t p_proc_pg_tbl = (pproc_pg_tbl_t)core_rtl_array_get(
                                           &usr_pg_tbls,
                                           current_proc);
        p_addr_map = &(p_proc_pg_tbl->free_addr_map);
        p_size_map = &(p_proc_pg_tbl->free_size_map);
        p_used_map = &(p_proc_pg_tbl->used_map);
    }

    //Allocate page
    ppage_block_t p_page_block = alloc_page(
                                     p_size_map,
                                     p_addr_map,
                                     p_used_map,
                                     base_addr,
                                     size);

    if(p_page_block == NULL) {
        goto EXCEPT_ALLOC;
    }

    if(options & PAGE_OPTION_WRITABLE) {
        p_page_block->status |= PAGE_BLOCK_WRITABLE;
    }

    if(options & PAGE_OPTION_EXECUTABLE) {
        p_page_block->status |= PAGE_BLOCK_EXECUTABLE;
    }

    if(options & PAGE_OPTION_SHARED) {
        p_page_block->status |= PAGE_BLOCK_SHARED;
    }

    if(options & PAGE_OPTION_COMMIT) {
        //Commit page
        if(!commit_page(p_page_block, options)) {
            goto EXCEPT_COMMIT;
        }
    }

    core_pm_spnlck_unlock(p_lock);
    return (void*)p_page_block->begin;
EXCEPT_COMMIT:
    free_page(p_page_block, p_size_map, p_addr_map, p_used_map);

EXCEPT_ALLOC:
    core_pm_spnlck_unlock(p_lock);

EXCEPT_CHECK_ADDR:
    return NULL;
}

void core_mm_pg_free(void* base_addr)
{
    address_t kernel_mem_base;
    size_t kernel_mem_size;
    address_t usr_mem_base;
    address_t usr_mem_size;

    //Get map
    hal_mmu_get_krnl_addr_range(
        (void**)(&kernel_mem_base),
        &kernel_mem_size);
    hal_mmu_get_usr_addr_range(
        (void**)(&usr_mem_base),
        &usr_mem_size);

    pmap_t p_addr_map;
    pmap_t p_used_map;
    pmap_t p_size_map;
    pspnlck_t p_lock;

    if((address_t)base_addr >= kernel_mem_base
       && (address_t)base_addr - kernel_mem_base
       < kernel_mem_size) {
        //Kernel memory
        p_lock = &krnl_pg_tbl_lck;
        p_addr_map = &krnl_pg_addr_map;
        p_size_map = &krnl_pg_size_map;
        p_used_map = &krnl_pg_used_map;

        core_pm_spnlck_lock(p_lock);

    } else if((address_t)base_addr >= usr_mem_base
              && (address_t)base_addr - usr_mem_base
              < usr_mem_size) {
        //User memory
        p_lock = &usr_pg_tbls_lck;
        u32 current_proc = core_pm_get_crrnt_proc_id();
        core_pm_spnlck_lock(p_lock);
        pproc_pg_tbl_t p_proc_pg_tbl = (pproc_pg_tbl_t)core_rtl_array_get(
                                           &usr_pg_tbls,
                                           current_proc);
        p_addr_map = &(p_proc_pg_tbl->free_addr_map);
        p_size_map = &(p_proc_pg_tbl->free_size_map);
        p_used_map = &(p_proc_pg_tbl->used_map);

    } else {
        //Raise exception
        char comment[64];
        core_rtl_snprintf(comment, sizeof(comment),
                          "Unable to free pages at %p, this address is not a "
                          "base address for an allocated page block.",
                          base_addr);
        peinval_except_t p_except = einval_except();
        RAISE(p_except, comment);

        return;
    }

    //Look for page block
    ppage_block_t p_page_block = core_rtl_map_search(
                                     p_used_map,
                                     (void*)base_addr,
                                     (map_search_func_t)search_addr,
                                     NULL);

    if(p_page_block == NULL) {
        core_pm_spnlck_unlock(p_lock);

        //Raise exception
        char comment[64];
        core_rtl_snprintf(comment, sizeof(comment),
                          "Unable to free pages at %p, this address is not a "
                          "base address for an allocated page block.",
                          base_addr);
        peinval_except_t p_except = einval_except();
        RAISE(p_except, comment);

        return;
    }

    //Free page block
    free_page(p_page_block, p_size_map, p_addr_map, p_used_map);

    core_pm_spnlck_unlock(p_lock);
    return;
}

ppage_obj_t core_mm_get_pg_obj(void** p_base_addr, void* addr)
{
    address_t kernel_mem_base;
    size_t kernel_mem_size;
    address_t usr_mem_base;
    address_t usr_mem_size;

    //Get map
    hal_mmu_get_krnl_addr_range(
        (void**)(&kernel_mem_base),
        &kernel_mem_size);
    hal_mmu_get_usr_addr_range(
        (void**)(&usr_mem_base),
        &usr_mem_size);

    pmap_t p_used_map;
    pspnlck_t p_lock;

    if((address_t)addr >= kernel_mem_base
       && (address_t)addr - kernel_mem_base
       < kernel_mem_size) {
        //Kernel memory
        p_lock = &krnl_pg_tbl_lck;
        p_used_map = &krnl_pg_used_map;

        core_pm_spnlck_lock(p_lock);

    } else if((address_t)addr >= usr_mem_base
              && (address_t)addr - usr_mem_base
              < usr_mem_size) {
        //User memory
        p_lock = &usr_pg_tbls_lck;
        u32 current_proc = core_pm_get_crrnt_proc_id();
        core_pm_spnlck_lock(p_lock);
        pproc_pg_tbl_t p_proc_pg_tbl = (pproc_pg_tbl_t)core_rtl_array_get(
                                           &usr_pg_tbls,
                                           current_proc);
        p_used_map = &(p_proc_pg_tbl->used_map);

    } else {
        return NULL;
    }

    //Look for page block
    ppage_block_t p_page_block = core_rtl_map_search(
                                     p_used_map,
                                     (void*)addr,
                                     (map_search_func_t)search_addr,
                                     NULL);

    if(p_page_block == NULL) {
        core_pm_spnlck_unlock(p_lock);
        return NULL;
    }

    *p_base_addr = (void*)(p_page_block->begin);

    ppage_obj_t p_ret = p_page_block->p_pg_obj;

    if(p_ret != NULL) {
        INC_REF(p_ret);
    }

    core_pm_spnlck_unlock(p_lock);
    return p_ret;
}

void* core_mm_map(void* addr, ppage_obj_t p_page_obj)
{
    address_t kernel_mem_base;
    size_t kernel_mem_size;
    address_t usr_mem_base;
    address_t usr_mem_size;

    //Get map
    hal_mmu_get_krnl_addr_range(
        (void**)(&kernel_mem_base),
        &kernel_mem_size);
    hal_mmu_get_usr_addr_range(
        (void**)(&usr_mem_base),
        &usr_mem_size);

    pmap_t p_used_map;
    pspnlck_t p_lock;

    if((address_t)addr >= kernel_mem_base
       && (address_t)addr - kernel_mem_base
       < kernel_mem_size) {
        //Kernel memory
        p_lock = &krnl_pg_tbl_lck;
        p_used_map = &krnl_pg_used_map;

        core_pm_spnlck_lock(p_lock);

    } else if((address_t)addr >= usr_mem_base
              && (address_t)addr - usr_mem_base
              < usr_mem_size) {
        //User memory
        p_lock = &usr_pg_tbls_lck;
        u32 current_proc = core_pm_get_crrnt_proc_id();
        core_pm_spnlck_lock(p_lock);
        pproc_pg_tbl_t p_proc_pg_tbl = (pproc_pg_tbl_t)core_rtl_array_get(
                                           &usr_pg_tbls,
                                           current_proc);
        p_used_map = &(p_proc_pg_tbl->used_map);

    } else {
        //Raise exception
        peinval_except_t p_except = einval_except();
        RAISE(p_except, NULL);
        return NULL;
    }

    //Look for page block
    ppage_block_t p_page_block = core_rtl_map_search(
                                     p_used_map,
                                     (void*)addr,
                                     (map_search_func_t)search_addr,
                                     NULL);

    if(p_page_block == NULL
       || p_page_block->begin != (address_t)addr
       || p_page_block->size != p_page_obj->get_size(p_page_obj)
       || p_page_block->status & PAGE_BLOCK_COMMITED) {
        core_pm_spnlck_unlock(p_lock);
        //Raise exception
        peinval_except_t p_except = einval_except();
        RAISE(p_except, NULL);
        return NULL;
    }

    //Commit page
    p_page_block->status |= PAGE_BLOCK_COMMITED;
    p_page_block->p_pg_obj = p_page_obj;

    u32 page_obj_attr = p_page_obj->get_attr(p_page_obj);

    if(page_obj_attr & PAGE_OBJ_ALLOCATED
       && page_obj_attr & PAGE_OBJ_SWAPPED) {
        p_page_obj->map(p_page_obj,
                        (void*)(p_page_block->begin),
                        p_page_block->status);
    }

    core_pm_spnlck_unlock(p_lock);
    return (void*)(p_page_block->begin);
}

void core_mm_commit(void* addr, u32 options)
{
    address_t kernel_mem_base;
    size_t kernel_mem_size;
    address_t usr_mem_base;
    address_t usr_mem_size;

    //Get map
    hal_mmu_get_krnl_addr_range(
        (void**)(&kernel_mem_base),
        &kernel_mem_size);
    hal_mmu_get_usr_addr_range(
        (void**)(&usr_mem_base),
        &usr_mem_size);

    pmap_t p_used_map;
    pspnlck_t p_lock;

    if((address_t)addr >= kernel_mem_base
       && (address_t)addr - kernel_mem_base
       < kernel_mem_size) {
        //Kernel memory
        p_lock = &krnl_pg_tbl_lck;
        p_used_map = &krnl_pg_used_map;

        core_pm_spnlck_lock(p_lock);

    } else if((address_t)addr >= usr_mem_base
              && (address_t)addr - usr_mem_base
              < usr_mem_size) {
        //User memory
        p_lock = &usr_pg_tbls_lck;
        u32 current_proc = core_pm_get_crrnt_proc_id();
        core_pm_spnlck_lock(p_lock);
        pproc_pg_tbl_t p_proc_pg_tbl = (pproc_pg_tbl_t)core_rtl_array_get(
                                           &usr_pg_tbls,
                                           current_proc);
        p_used_map = &(p_proc_pg_tbl->used_map);

    } else {
        //Raise exception
        peinval_except_t p_except = einval_except();
        RAISE(p_except, NULL);

        return;
    }

    //Look for page block
    ppage_block_t p_page_block = core_rtl_map_search(
                                     p_used_map,
                                     (void*)addr,
                                     (map_search_func_t)search_addr,
                                     NULL);

    if(p_page_block == NULL
       || p_page_block->begin != (address_t)addr
       || p_page_block->status & PAGE_BLOCK_COMMITED) {
        core_pm_spnlck_unlock(p_lock);
        //Raise exception
        peinval_except_t p_except = einval_except();
        RAISE(p_except, NULL);
        return;
    }

    if(!commit_page(p_page_block, options)) {
        core_pm_spnlck_unlock(p_lock);
        //Raise exception
        peinval_except_t p_except = einval_except();
        RAISE(p_except, NULL);
        return;
    }

    core_pm_spnlck_unlock(p_lock);
    return;
}

void core_mm_uncommit(void* addr)
{
    address_t kernel_mem_base;
    size_t kernel_mem_size;
    address_t usr_mem_base;
    address_t usr_mem_size;

    //Get map
    hal_mmu_get_krnl_addr_range(
        (void**)(&kernel_mem_base),
        &kernel_mem_size);
    hal_mmu_get_usr_addr_range(
        (void**)(&usr_mem_base),
        &usr_mem_size);

    pmap_t p_used_map;
    pspnlck_t p_lock;

    if((address_t)addr >= kernel_mem_base
       && (address_t)addr - kernel_mem_base
       < kernel_mem_size) {
        //Kernel memory
        p_lock = &krnl_pg_tbl_lck;
        p_used_map = &krnl_pg_used_map;

        core_pm_spnlck_lock(p_lock);

    } else if((address_t)addr >= usr_mem_base
              && (address_t)addr - usr_mem_base
              < usr_mem_size) {
        //User memory
        p_lock = &usr_pg_tbls_lck;
        u32 current_proc = core_pm_get_crrnt_proc_id();
        core_pm_spnlck_lock(p_lock);
        pproc_pg_tbl_t p_proc_pg_tbl = (pproc_pg_tbl_t)core_rtl_array_get(
                                           &usr_pg_tbls,
                                           current_proc);
        p_used_map = &(p_proc_pg_tbl->used_map);

    } else {
        //Raise exception
        peinval_except_t p_except = einval_except();
        RAISE(p_except, NULL);
        return;
    }

    //Look for page block
    ppage_block_t p_page_block = core_rtl_map_search(
                                     p_used_map,
                                     (void*)addr,
                                     (map_search_func_t)search_addr,
                                     NULL);

    if(p_page_block == NULL
       || p_page_block->begin != (address_t)addr
       || !(p_page_block->status & PAGE_BLOCK_COMMITED)) {
        core_pm_spnlck_unlock(p_lock);
        //Raise exception
        peinval_except_t p_except = einval_except();
        RAISE(p_except, NULL);
        return;
    }

    //Uncommit page block
    p_page_block->p_pg_obj->unmap(
        p_page_block->p_pg_obj,
        addr);
    DEC_REF(p_page_block->p_pg_obj);
    p_page_block->p_pg_obj = NULL;
    p_page_block->status &= ~PAGE_BLOCK_COMMITED;

    core_pm_spnlck_unlock(p_lock);
    return;
}

u32 core_mm_get_pg_attr(void* address)
{
    u32 ret = 0;

    address_t kernel_mem_base;
    size_t kernel_mem_size;
    address_t usr_mem_base;
    address_t usr_mem_size;

    //Get map
    hal_mmu_get_krnl_addr_range(
        (void**)(&kernel_mem_base),
        &kernel_mem_size);
    hal_mmu_get_usr_addr_range(
        (void**)(&usr_mem_base),
        &usr_mem_size);

    pmap_t p_used_map;
    pspnlck_t p_lock;

    if((address_t)address >= kernel_mem_base
       && (address_t)address - kernel_mem_base
       < kernel_mem_size) {
        //Kernel memory
        p_lock = &krnl_pg_tbl_lck;
        p_used_map = &krnl_pg_used_map;

        core_pm_spnlck_lock(p_lock);
        ret |= PAGE_OPTION_KERNEL;

    } else if((address_t)address >= usr_mem_base
              && (address_t)address - usr_mem_base
              < usr_mem_size) {
        //User memory
        p_lock = &usr_pg_tbls_lck;
        u32 current_proc = core_pm_get_crrnt_proc_id();
        core_pm_spnlck_lock(p_lock);
        pproc_pg_tbl_t p_proc_pg_tbl = (pproc_pg_tbl_t)core_rtl_array_get(
                                           &usr_pg_tbls,
                                           current_proc);
        p_used_map = &(p_proc_pg_tbl->used_map);

    } else {
        return 0;
    }

    //Look for page block
    ppage_block_t p_page_block = core_rtl_map_search(
                                     p_used_map,
                                     (void*)address,
                                     (map_search_func_t)search_addr,
                                     NULL);

    if(p_page_block == NULL) {
        core_pm_spnlck_unlock(p_lock);
        return ret;
    }

    //Get page block attributes
    if(p_page_block->status & PAGE_BLOCK_WRITABLE) {
        ret |= PAGE_OPTION_WRITABLE;
    }

    if(p_page_block->status & PAGE_BLOCK_EXECUTABLE) {
        ret |= PAGE_OPTION_EXECUTABLE;
    }

    if(p_page_block->status & PAGE_BLOCK_SHARED) {
        ret |= PAGE_OPTION_SHARED;
    }

    if(p_page_block->status & PAGE_BLOCK_COMMITED) {
        ret |= PAGE_OPTION_COMMIT;
        //Get page oject attributes
        ppage_obj_t p_page_obj = p_page_block->p_pg_obj;
        u32 attr = p_page_obj->get_attr(p_page_obj);

        if(attr & PAGE_OBJ_SWAPPABLE) {
            ret |= PAGE_OPTION_SWAPPABLE;
        }

        if(attr & PAGE_OBJ_DMA) {
            ret |= PAGE_OPTION_DMA;
        }

        if(attr & PAGE_OBJ_CAHCEABLE) {
            ret |= PAGE_OPTION_CACHEABLE;
        }
    }

    core_pm_spnlck_unlock(p_lock);
    return ret;
}

u32 core_mm_set_pg_attr(void* address, u32 attr)
{
    u32 ret = 0;

    address_t kernel_mem_base;
    size_t kernel_mem_size;
    address_t usr_mem_base;
    address_t usr_mem_size;

    //Get map
    hal_mmu_get_krnl_addr_range(
        (void**)(&kernel_mem_base),
        &kernel_mem_size);
    hal_mmu_get_usr_addr_range(
        (void**)(&usr_mem_base),
        &usr_mem_size);

    pmap_t p_used_map;
    pspnlck_t p_lock;

    if((address_t)address >= kernel_mem_base
       && (address_t)address - kernel_mem_base
       < kernel_mem_size) {
        //Kernel memory
        p_lock = &krnl_pg_tbl_lck;
        p_used_map = &krnl_pg_used_map;

        core_pm_spnlck_lock(p_lock);
        ret |= PAGE_OPTION_KERNEL;

    } else if((address_t)address >= usr_mem_base
              && (address_t)address - usr_mem_base
              < usr_mem_size) {
        //User memory
        p_lock = &usr_pg_tbls_lck;
        u32 current_proc = core_pm_get_crrnt_proc_id();
        core_pm_spnlck_lock(p_lock);
        pproc_pg_tbl_t p_proc_pg_tbl = (pproc_pg_tbl_t)core_rtl_array_get(
                                           &usr_pg_tbls,
                                           current_proc);
        p_used_map = &(p_proc_pg_tbl->used_map);

    } else {
        return 0;
    }

    //Look for page block
    ppage_block_t p_page_block = core_rtl_map_search(
                                     p_used_map,
                                     (void*)address,
                                     (map_search_func_t)search_addr,
                                     NULL);

    if(p_page_block == NULL) {
        core_pm_spnlck_unlock(p_lock);
        return ret;
    }

    //Set attribute
    if(attr & PAGE_OPTION_WRITABLE) {
        p_page_block->status |= PAGE_BLOCK_WRITABLE;

    } else {
        p_page_block->status &= ~PAGE_BLOCK_WRITABLE;
    }

    if(attr & PAGE_OPTION_EXECUTABLE) {
        p_page_block->status |= PAGE_BLOCK_EXECUTABLE;

    } else {
        p_page_block->status &= ~PAGE_BLOCK_EXECUTABLE;
    }

    p_page_block->p_pg_obj->map(
        p_page_block->p_pg_obj,
        (void*)(p_page_block->begin),
        p_page_block->status);

    //Get page block attributes
    if(p_page_block->status & PAGE_BLOCK_WRITABLE) {
        ret |= PAGE_OPTION_WRITABLE;
    }

    if(p_page_block->status & PAGE_BLOCK_EXECUTABLE) {
        ret |= PAGE_OPTION_EXECUTABLE;
    }

    if(p_page_block->status & PAGE_BLOCK_SHARED) {
        ret |= PAGE_OPTION_SHARED;
    }

    if(p_page_block->status & PAGE_BLOCK_COMMITED) {
        ret |= PAGE_OPTION_COMMIT;
        //Get page oject attributes
        ppage_obj_t p_page_obj = p_page_block->p_pg_obj;
        u32 obj_attr = p_page_obj->get_attr(p_page_obj);

        if(obj_attr & PAGE_OBJ_SWAPPABLE) {
            ret |= PAGE_OPTION_SWAPPABLE;
        }

        if(obj_attr & PAGE_OBJ_DMA) {
            ret |= PAGE_OPTION_DMA;
        }

        if(obj_attr & PAGE_OBJ_CAHCEABLE) {
            ret |= PAGE_OPTION_CACHEABLE;
        }
    }

    core_pm_spnlck_unlock(p_lock);
    return ret;
}

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
                              "%-16s | %-16s | %-10s\n",
                              "Virtual address",
                              "Physical address",
                              "Size");

    while(hal_mmu_get_next_mapped_pages((void*)base_addr,
                                        (void**)(&base_addr),
                                        (void**)(&phy_begin),
                                        &size, &attr)) {
        core_kconsole_print_debug("%-16p | %-16p | %-10p\n",
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
        p_page_block->status |= PAGE_BLOCK_COMMITED;

        if(attr & MMU_PAGE_WRITABLE) {
            p_page_block->status |= PAGE_WRITABLE;

        } else if(attr & MMU_PAGE_EXECUTABLE) {
            p_page_block->status |= PAGE_EXECUTABLE;
        }

        base_addr = (address_t)base_addr + size;
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

    HEAP_CHECK(paging_heap);

    if(core_rtl_map_set(
           &(p_page_tbl_0->free_addr_map),
           p_page_block,
           p_page_block) == NULL) {
        PANIC(ENOMEM, "Failed to add first user page block.\n");
    }

    HEAP_CHECK(paging_heap);

    if(core_rtl_map_set(
           &(p_page_tbl_0->free_size_map),
           p_page_block,
           p_page_block) == NULL) {
        PANIC(ENOMEM, "Failed to add first user page block.\n");
    }

    //Add page table
    HEAP_CHECK(paging_heap);

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
          && !(p_block->p_prev->status & PAGE_BLOCK_ALLOCATED)) {
        p_block = p_block->p_prev;
    }

    //Merge blocks
    //Remove p_block from map
    core_rtl_map_set(p_size_map, p_block, NULL);
    core_rtl_map_set(p_addr_map, p_block, NULL);

    while(p_block->p_next != NULL
          && !(p_block->p_next->status & PAGE_BLOCK_ALLOCATED)) {
        //Remove next block from map
        ppage_block_t p_next = p_block->p_next;
        core_rtl_map_set(p_size_map, p_next, NULL);
        core_rtl_map_set(p_addr_map, p_next, NULL);

        //Merge block
        p_block->size += p_next->size;

        if(p_next->p_next != NULL) {
            p_next->p_next->p_prev = p_block;
        }

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
            if(p_page_block->size < size
               || p_page_block->size - size
               < (address_t)base_addr - p_page_block->begin) {
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
        p_page_block->begin += p_prev_block->size;
        p_page_block->size -= p_prev_block->size;

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
    p_page_block->status = PAGE_BLOCK_ALLOCATED;
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

bool commit_page(ppage_block_t p_page_block, u32 options)
{
    u32 attr = 0;

    //Get page attributes
    if(options & PAGE_OPTION_DMA) {
        attr |= PAGE_OBJ_DMA;
    }

    if(options & PAGE_OPTION_CACHEABLE) {
        attr |= PAGE_OPTION_CACHEABLE;
    }

    if(options & PAGE_OPTION_SWAPPABLE) {
        attr |= PAGE_OPTION_SWAPPABLE;
    }

    //Create page object
    ppage_obj_t p_page_obj = page_obj(p_page_block->size,
                                      attr);

    if(p_page_obj == NULL) {
        return false;
    }

    p_page_block->p_pg_obj = p_page_obj;
    p_page_block->status |= PAGE_BLOCK_COMMITED;

    if(options & PAGE_OPTION_ALLOCWHENCOMMIT) {
        p_page_obj->alloc(p_page_obj);
        p_page_obj->map(p_page_obj,
                        (void*)p_page_block->begin,
                        p_page_block->status);
    }

    return true;
}

void free_page(ppage_block_t p_block, pmap_t p_size_map, pmap_t p_addr_map,
               pmap_t p_used_map)
{
    //Remove for used map
    core_rtl_map_set(p_used_map, p_block, NULL);

    //Free page object
    if(p_block->p_pg_obj != NULL) {
        p_block->p_pg_obj->unmap(p_block->p_pg_obj,
                                 (void*)p_block->begin);
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

void clear_usr_pg_tbl(pproc_pg_tbl_t p_usr_tbl)
{
    for(ppage_block_t p_pg_blk = core_rtl_map_next(
                                     &(p_usr_tbl->used_map),
                                     NULL);
        p_pg_blk != NULL;
        p_pg_blk = core_rtl_map_next(
                       &(p_usr_tbl->used_map),
                       NULL)) {
        free_page(p_pg_blk,
                  &(p_usr_tbl->free_size_map),
                  &(p_usr_tbl->free_addr_map),
                  &(p_usr_tbl->used_map));
    }

    return;
}

int	search_size(size_t size, ppage_block_t p_key, ppage_block_t p_value,
                ppage_block_t* p_p_prev)
{
    ppage_block_t p_page_block = (ppage_block_t)p_value;

    if(size > p_page_block->size) {
        return 1;

    } else if(size < p_page_block->size) {
        *p_p_prev = p_page_block;
        return -1;

    } else {
        *p_p_prev = p_page_block;
        return 0;
    }

    UNREFERRED_PARAMETER(p_key);
}

int search_addr(address_t address, ppage_block_t p_key, ppage_block_t p_value,
                void* p_args)
{
    if(address < p_key->begin) {
        return -1;

    } else if(address - p_key->begin >= p_key->size) {
        return 1;

    } else {
        return 0;
    }

    UNREFERRED_PARAMETER(p_value);
    UNREFERRED_PARAMETER(p_args);
}

except_stat_t page_read_except_hndlr(except_reason_t reason,
                                     pepageread_except_t p_except)
{
    //Push dead lock handler
    except_hndlr_info_t dead_lck_hndlr_info = {
        .hndlr = (except_hndlr_t)deadlock_except_hndlr,
        .reason = EDEADLOCK
    };

    if(core_exception_push_hndlr(&dead_lck_hndlr_info) == EXCEPT_RET_UNWIND) {
        //EDEADLOCK occured, page fault in paging module
        pepfinpaging_except_t p_new_except = epfinpaging_except(
                p_except->fault_addr);
        p_new_except->except.raise((pexcept_obj_t)p_new_except,
                                   p_except->except.p_context,
                                   __FILE__, __LINE__, NULL);
    }

    //Check if the address is in kernel memory or user memory.
    address_t usr_base;
    size_t usr_mem_size;

    hal_mmu_get_usr_addr_range((void**)(&usr_base),
                               &usr_mem_size);

    pspnlck_t p_lock;
    pmap_t p_used_map;

    if(p_except->fault_addr >= usr_base
       && p_except->fault_addr < usr_base + usr_mem_size) {
        //User memory
        p_lock = &usr_pg_tbls_lck;
        core_pm_spnlck_lock(p_lock);
        pproc_pg_tbl_t p_pg_tbl = (pproc_pg_tbl_t)core_rtl_array_get(
                                      &usr_pg_tbls,
                                      core_pm_get_currnt_thread_id());
        p_used_map = &(p_pg_tbl->used_map);

    } else {
        //Kernel memory
        p_lock = &krnl_pg_tbl_lck;
        core_pm_spnlck_lock(p_lock);
        p_used_map = &krnl_pg_used_map;
    }

    //Search page block
    ppage_block_t p_page_block = core_rtl_map_search(
                                     p_used_map,
                                     (void*)(p_except->fault_addr),
                                     (map_search_func_t)search_addr,
                                     NULL);

    if(p_page_block == NULL) {
        goto EXCEPT_OCCURED;
    }

    //Check if the page block has been commited
    if(p_page_block->status & PAGE_BLOCK_COMMITED) {
        //Check page object attributes
        ppage_obj_t p_pg_obj = p_page_block->p_pg_obj;
        u32 attr = p_pg_obj->get_attr(p_pg_obj);

        if(!(attr & PAGE_OBJ_ALLOCATED)) {
            //Allocate page
            p_pg_obj->alloc(p_pg_obj);
            p_pg_obj->map(p_pg_obj, (void*)(p_page_block->begin),
                          p_page_block->status);

        } else if(attr & PAGE_OBJ_SWAPPED) {
            //Unswap page
            p_pg_obj->unswap(p_pg_obj);
            p_pg_obj->map(p_pg_obj, (void*)(p_page_block->begin),
                          p_page_block->status);

        } else {
            //Map page object
            p_pg_obj->map(p_pg_obj, (void*)(p_page_block->begin),
                          p_page_block->status);

        }

    } else {
        goto EXCEPT_OCCURED;
    }

    core_pm_spnlck_unlock(p_lock);
    core_exception_pop_hndlr();

    return EXCEPT_STATUS_CONTINUE_EXEC;

    //Exception
EXCEPT_OCCURED:
    core_pm_spnlck_unlock(p_lock);
    core_exception_pop_hndlr();
    return EXCEPT_STATUS_CONTINUE_SEARCH;

    UNREFERRED_PARAMETER(reason);
}

except_stat_t page_write_except_hndlr(except_reason_t reason,
                                      pepagewrite_except_t p_except)
{
    //Push dead lock handler
    except_hndlr_info_t dead_lck_hndlr_info = {
        .hndlr = (except_hndlr_t)deadlock_except_hndlr,
        .reason = EDEADLOCK
    };

    if(core_exception_push_hndlr(&dead_lck_hndlr_info) == EXCEPT_RET_UNWIND) {
        //EDEADLOCK occured, page fault in paging module
        pepfinpaging_except_t p_new_except = epfinpaging_except(
                p_except->fault_addr);
        p_new_except->except.raise((pexcept_obj_t)p_new_except,
                                   p_except->except.p_context,
                                   __FILE__, __LINE__, NULL);
    }

    //Check if the address is in kernel memory or user memory.
    address_t usr_base;
    size_t usr_mem_size;

    hal_mmu_get_usr_addr_range((void**)(&usr_base),
                               &usr_mem_size);

    pspnlck_t p_lock;
    pmap_t p_used_map;

    if(p_except->fault_addr >= usr_base
       && p_except->fault_addr < usr_base + usr_mem_size) {
        //User memory
        p_lock = &usr_pg_tbls_lck;
        core_pm_spnlck_lock(p_lock);
        pproc_pg_tbl_t p_pg_tbl = (pproc_pg_tbl_t)core_rtl_array_get(
                                      &usr_pg_tbls,
                                      core_pm_get_currnt_thread_id());
        p_used_map = &(p_pg_tbl->used_map);

    } else {
        //Kernel memory
        p_lock = &krnl_pg_tbl_lck;
        core_pm_spnlck_lock(p_lock);
        p_used_map = &krnl_pg_used_map;
    }

    //Search page block
    ppage_block_t p_page_block = core_rtl_map_search(
                                     p_used_map,
                                     (void*)(p_except->fault_addr),
                                     (map_search_func_t)search_addr,
                                     NULL);

    if(p_page_block == NULL) {
        goto EXCEPT_OCCURED;
    }

    //Check if the page block has been commited
    if(p_page_block->status & PAGE_BLOCK_COMMITED) {
        //Check if the page can be write
        if(!(p_page_block->status & PAGE_BLOCK_WRITABLE)) {
            goto EXCEPT_OCCURED;
        }

        //Check page object attributes
        ppage_obj_t p_pg_obj = p_page_block->p_pg_obj;
        u32 attr = p_pg_obj->get_attr(p_pg_obj);

        if(!(attr & PAGE_OBJ_ALLOCATED)) {
            //Allocate page
            p_pg_obj->alloc(p_pg_obj);
            p_pg_obj->map(p_pg_obj, (void*)(p_page_block->begin),
                          p_page_block->status);

        } else if(attr & PAGE_OBJ_COPY_ON_WRITE) {
            //Copy on write
            p_pg_obj->copy_on_write(p_pg_obj, (void*)(p_page_block->begin),
                                    p_page_block->status);

        } else if(attr & PAGE_OBJ_SWAPPED) {
            //Unswap page
            p_pg_obj->unswap(p_pg_obj);
            p_pg_obj->map(p_pg_obj, (void*)(p_page_block->begin),
                          p_page_block->status);

        } else {
            //Map page object
            p_pg_obj->map(p_pg_obj, (void*)(p_page_block->begin),
                          p_page_block->status);

        }

    } else {
        goto EXCEPT_OCCURED;
    }

    core_pm_spnlck_unlock(p_lock);
    core_exception_pop_hndlr();

    return EXCEPT_STATUS_CONTINUE_EXEC;

    //Exception
EXCEPT_OCCURED:
    core_pm_spnlck_unlock(p_lock);
    core_exception_pop_hndlr();
    return EXCEPT_STATUS_CONTINUE_SEARCH;

    UNREFERRED_PARAMETER(reason);
}

except_stat_t page_exec_except_hndlr(except_reason_t reason,
                                     pepageexec_except_t p_except)
{
    //Push dead lock handler
    except_hndlr_info_t dead_lck_hndlr_info = {
        .hndlr = (except_hndlr_t)deadlock_except_hndlr,
        .reason = EDEADLOCK
    };

    if(core_exception_push_hndlr(&dead_lck_hndlr_info) == EXCEPT_RET_UNWIND) {
        //EDEADLOCK occured, page fault in paging module
        pepfinpaging_except_t p_new_except = epfinpaging_except(
                p_except->fault_addr);
        p_new_except->except.raise((pexcept_obj_t)p_new_except,
                                   p_except->except.p_context,
                                   __FILE__, __LINE__, NULL);
    }

    //Check if the address is in kernel memory or user memory.
    address_t usr_base;
    size_t usr_mem_size;

    hal_mmu_get_usr_addr_range((void**)(&usr_base),
                               &usr_mem_size);

    pspnlck_t p_lock;
    pmap_t p_used_map;

    if(p_except->fault_addr >= usr_base
       && p_except->fault_addr < usr_base + usr_mem_size) {
        //User memory
        p_lock = &usr_pg_tbls_lck;
        core_pm_spnlck_lock(p_lock);
        pproc_pg_tbl_t p_pg_tbl = (pproc_pg_tbl_t)core_rtl_array_get(
                                      &usr_pg_tbls,
                                      core_pm_get_currnt_thread_id());
        p_used_map = &(p_pg_tbl->used_map);

    } else {
        //Kernel memory
        p_lock = &krnl_pg_tbl_lck;
        core_pm_spnlck_lock(p_lock);
        p_used_map = &krnl_pg_used_map;
    }

    //Search page block
    ppage_block_t p_page_block = core_rtl_map_search(
                                     p_used_map,
                                     (void*)(p_except->fault_addr),
                                     (map_search_func_t)search_addr,
                                     NULL);

    if(p_page_block == NULL) {
        goto EXCEPT_OCCURED;
    }

    //Check if the page block has been commited
    if(p_page_block->status & PAGE_BLOCK_COMMITED) {
        //Check if the page can be executed
        if(!(p_page_block->status & PAGE_BLOCK_EXECUTABLE)) {
            goto EXCEPT_OCCURED;
        }

        //Check page object attributes
        ppage_obj_t p_pg_obj = p_page_block->p_pg_obj;
        u32 attr = p_pg_obj->get_attr(p_pg_obj);

        if(!(attr & PAGE_OBJ_ALLOCATED)) {
            //Allocate page
            p_pg_obj->alloc(p_pg_obj);
            p_pg_obj->map(p_pg_obj, (void*)(p_page_block->begin),
                          p_page_block->status);

        } else if(attr & PAGE_OBJ_SWAPPED) {
            //Unswap page
            p_pg_obj->unswap(p_pg_obj);
            p_pg_obj->map(p_pg_obj, (void*)(p_page_block->begin),
                          p_page_block->status);

        } else {
            //Map page object
            p_pg_obj->map(p_pg_obj, (void*)(p_page_block->begin),
                          p_page_block->status);
        }

    } else {
        goto EXCEPT_OCCURED;
    }

    core_pm_spnlck_unlock(p_lock);
    core_exception_pop_hndlr();

    return EXCEPT_STATUS_CONTINUE_EXEC;

    //Exception
EXCEPT_OCCURED:
    core_pm_spnlck_unlock(p_lock);
    core_exception_pop_hndlr();
    return EXCEPT_STATUS_CONTINUE_SEARCH;

    UNREFERRED_PARAMETER(reason);
}

except_stat_t deadlock_except_hndlr(except_reason_t reason,
                                    pedeadlock_except_t p_except)
{
    UNREFERRED_PARAMETER(reason);
    UNREFERRED_PARAMETER(p_except);

    return EXCEPT_STATUS_UNWIND;
}
