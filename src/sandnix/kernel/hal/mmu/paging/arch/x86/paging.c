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

#include "../../paging.h"
#include "../../../mmu.h"
#include "../../../../../core/pm/pm.h"
#include "../../../../../core/mm/mm.h"
#include "../../../../../core/rtl/rtl.h"
#include "../../../../init/init.h"
#include "../../../../exception/exception.h"
#include "../../../../early_print/early_print.h"

#define MODULE_NAME	hal_mmu

#if	KERNEL_MEM_BASE % (4096 * 1024) !=0
    #error	"KERNEL_MEM_BASE must be 4MB aligned."
#endif


#define	REQUIRED_INIT_PAGE_NUM	((1024 * 1024 + KERNEL_MAX_SIZE) / 4096)
#define	MAX_INIT_PAGE_NUM		(REQUIRED_INIT_PAGE_NUM % 1024 > 0 \
                                 ? (REQUIRED_INIT_PAGE_NUM / 1024 + 1) * 1024 \
                                 : REQUIRED_INIT_PAGE_NUM)


static pde_t __attribute__((aligned(4096)))	init_pde_tbl[1024];
static pte_t __attribute__((aligned(4096)))	init_pte_tbl[MAX_INIT_PAGE_NUM];
static u32			used_pte_table;
static address_t	load_offset;
static u32			mapped_init_page;

static array_t		mmu_globl_pg_table;
static map_t		mmu_krnl_pte_tbl_map;
static pheap_t		mmu_paging_heap;
static u8			mmu_paging_heap_block[4096];
static void*		page_operate_addr;
static spnlck_rw_t	lock;

static bool		initialized = false;


static int			compare_vaddr(address_t p1, address_t p2);
static void			krnl_pdt_sync(ppg_tbl_info_t p_pdt_info);
static void			switch_editing_page(void* phy_addr);
static void			create_0();
static void			refresh_TLB(void* address);
static void			remove_page(ppg_tbl_info_t p_pdt_info, void* virt_addr);
static kstatus_t	set_page(ppg_tbl_info_t p_pdt_info, void* virt_addr,
                             u32 attr, void* phy_addr);
static void			pte_info_destroy(ppte_tbl_info_t p_pte_info, void* p_null);

void start_paging()
{
    address_t offset = 0;
    ppde_t p_init_pde_tbl;
    ppde_t p_pde;
    ppte_t p_init_pte_tbl;
    ppte_t p_pte;
    pkrnl_hdr_t p_krnl_header;
    size_t total_pg_num;
    size_t total_map_size;
    u32 pde_num;
    u32 empty_pte_num;

    /*
     * When this function is called, the paging has not been started.We need to
     * compute the offset in order to get the physical address of global
     * variables.
     */
    //Compute the offset.
    __asm__ __volatile__(
        "call	_ADDR\n"
        "_ADDR:\n"
        "popl	%0\n"
        "subl	$_ADDR,%0\n"
        :"=ax"(offset)
        ::"memory");

    if(offset == 0) {
        PANIC(ENOTSUP,
              "Function start_paging() can only be used when mmu module "
              "has not been initialized");
    }


    //Compute real address
    p_init_pde_tbl = (ppde_t)((address_t)(&init_pde_tbl) + offset);
    p_init_pte_tbl = (ppte_t)((address_t)(&init_pte_tbl) + offset);
    p_krnl_header = (pkrnl_hdr_t)((address_t)(&kernel_header) + offset);

    /*
     * We just need to map the first 1MB memory and the memory where the kernel
     * image are to make sure the initializing code can run and early print can
     * work. Other work should be done by the initialize function of the mmu
     * module
     */
    //Compute how many pages to map
    total_map_size = (address_t)(p_krnl_header->code_start)
                     + p_krnl_header->code_size;

    if(total_map_size < (address_t)(p_krnl_header->data_start)
       + p_krnl_header->data_size) {
        total_map_size = (address_t)(p_krnl_header->data_start)
                         + p_krnl_header->data_size;
    }

    total_map_size -= KERNEL_MEM_BASE;

    total_pg_num = total_map_size / 4096;

    if(total_map_size % 4096 != 0) {
        total_pg_num++;
    }

    //Create initialize page table
    p_pte = p_init_pte_tbl;
    empty_pte_num = total_pg_num % 1024;

    if(empty_pte_num > 0) {
        empty_pte_num = 1024 - empty_pte_num;
    }

    //Fill page table
    for(u32 i = 0; i < total_pg_num; i++) {
        p_pte->present = PG_P;
        p_pte->read_write = PG_RW;
        p_pte->user_supervisor = PG_SUPERVISOR;
        p_pte->write_through = PG_WRITE_THROUGH;
        p_pte->cache_disabled = PG_ENCACHE;
        p_pte->accessed = 0;
        p_pte->dirty = 0;
        p_pte->page_table_attr_index = 0;
        p_pte->global_page = 0;
        p_pte->avail = 0;
        p_pte->page_base_addr = i;

        p_pte++;
    }

    for(u32 i = 0; i < empty_pte_num; i++) {
        p_pte->present = PG_NP;
        p_pte++;
    }

    //Fill PDT
    p_pde = p_init_pde_tbl;
    pde_num = total_pg_num / 1024;

    if(total_pg_num % 1024 > 0) {
        pde_num++;
    }

    for(u32 i = 0; i < 1024; i++) {
        if(i < pde_num) {
            p_pde->present = PG_P;
            p_pde->read_write = PG_RW;
            p_pde->user_supervisor = PG_SUPERVISOR;
            p_pde->write_through = PG_WRITE_THROUGH;
            p_pde->cache_disabled = PG_ENCACHE;
            p_pde->accessed = 0;
            p_pde->reserved = 0;
            p_pde->page_size = PG_SIZE_4K;
            p_pde->global_page = 0;
            p_pde->avail = 0;
            p_pde->page_table_base_addr = ((address_t)(p_init_pte_tbl + (i << 10))) >> 12;

        } else if(i >= (KERNEL_MEM_BASE >> 22)
                  && i < (KERNEL_MEM_BASE >> 22) + pde_num) {
            p_pde->present = PG_P;
            p_pde->read_write = PG_RW;
            p_pde->user_supervisor = PG_SUPERVISOR;
            p_pde->write_through = PG_WRITE_THROUGH;
            p_pde->cache_disabled = PG_ENCACHE;
            p_pde->accessed = 0;
            p_pde->reserved = 0;
            p_pde->page_size = PG_SIZE_4K;
            p_pde->global_page = 0;
            p_pde->avail = 0;
            p_pde->page_table_base_addr = ((address_t)(p_init_pte_tbl +
                                           ((i - (KERNEL_MEM_BASE >> 22)) << 10))) >> 12;

        } else {
            p_pde->present = PG_NP;
        }

        p_pde++;
    }

    //Load cr3
    __asm__ __volatile__(
        "andl	$0xFFFFF000,%0\n"
        "orl	$0x008,%0\n"
        "movl	%0, %%cr3\n"
        ::"eax"(p_init_pde_tbl)
        :"memory");

    //Start paging
    __asm__ __volatile__(
        "movl	%%cr0, %%eax\n"
        "orl	$0x80010000, %%eax\n"
        "movl	%%eax, %%cr0\n"
        :::"eax", "memory");

    used_pte_table = ((total_pg_num + empty_pte_num) >> 10);	//2^10 == 1024
    load_offset = offset;
    mapped_init_page = total_pg_num;

    return;
}

void* hal_mmu_add_early_paging_addr(void* phy_addr, u32 attr)
{
    ppde_t p_pde;
    ppte_t p_pte;

    if(initialized) {
        PANIC(ENOTSUP,
              "Function hal_mmu_add_early_paging_addr() can only "
              "be used when mmu module has not been initialized");
    }

    if(!(attr & MMU_PAGE_AVAIL)) {
        PANIC(EINVAL,
              "Cannot add an unavailable page!\n");
    }

    if((address_t)phy_addr < mapped_init_page * 4096) {
        return (void*)((address_t)phy_addr + KERNEL_MEM_BASE);
    }

    for(p_pte = init_pte_tbl + mapped_init_page;
        p_pte->present == PG_P;
        p_pte++) {
        if((p_pte->page_base_addr << 12) == (address_t)phy_addr) {
            //The address has been mapped
            return (void*)((p_pte - init_pte_tbl) * 4096 + KERNEL_MEM_BASE);
        }
    }

    //Allocate new page
    p_pte->present = PG_P;

    if(attr & MMU_PAGE_WRITABLE) {
        p_pte->read_write = PG_RW;

    } else {
        p_pte->read_write = PG_RDONLY;
    }

    p_pte->user_supervisor = PG_SUPERVISOR;
    p_pte->write_through = PG_WRITE_THROUGH;

    if(attr & MMU_PAGE_CACHEABLE) {
        p_pte->cache_disabled = PG_ENCACHE;

    } else {
        p_pte->cache_disabled =  PG_DISCACHE;
    }

    p_pte->accessed = 0;
    p_pte->dirty = 0;
    p_pte->page_table_attr_index = 0;
    p_pte->global_page = 0;
    p_pte->avail = 0;
    p_pte->page_base_addr = ((address_t)phy_addr) >> 12;

    if((p_pte - init_pte_tbl) % 1024 == 0) {
        p_pde = init_pde_tbl + (p_pte - init_pte_tbl) / 1024;
        p_pde->present = PG_P;
        p_pde->read_write = PG_RW;
        p_pde->user_supervisor = PG_SUPERVISOR;
        p_pde->write_through = PG_WRITE_THROUGH;
        p_pde->cache_disabled = PG_ENCACHE;
        p_pde->accessed = 0;
        p_pde->reserved = 0;
        p_pde->page_size = PG_SIZE_4K;
        p_pde->global_page = 0;
        p_pde->avail = 0;
        p_pde->page_table_base_addr = ((address_t)p_pte + load_offset) >> 12;
        used_pte_table++;
    }

    void* ret = (void*)((p_pte - init_pte_tbl) * 4096 + KERNEL_MEM_BASE);
    refresh_TLB(ret);

    return ret;
}

void PRIVATE(paging_init)()
{
    hal_early_print_printf("\nInitializing paging module of mmu...\n");
    mmu_paging_heap = core_mm_heap_create_on_buf(
                          HEAP_MULITHREAD | HEAP_PREALLOC,
                          4096, mmu_paging_heap_block,
                          sizeof(mmu_paging_heap_block));

    if(mmu_paging_heap == NULL) {
        PANIC(ENOMEM,
              "Not enough memory for mmu paging managment.");
    }

    if(core_rtl_array_init(&mmu_globl_pg_table, MAX_PROCESS_NUM,
                           mmu_paging_heap) != ESUCCESS) {
        PANIC(ENOMEM,
              "Not enough memory for mmu paging managment.");
    }

    core_rtl_map_init(&mmu_krnl_pte_tbl_map, (item_compare_t)(compare_vaddr),
                      mmu_paging_heap);
    //Create process 0 page table
    hal_early_print_printf("Creating paging table 0...\n");
    create_0();

    //Initialize lock
    core_pm_spnlck_rw_init(&lock);

    //Switch to page 0
    hal_early_print_printf("Switching to page table 0...\n");
    hal_mmu_pg_tbl_switch(0);
    initialized = true;
    return;
}

void hal_mmu_get_krnl_addr_range(void** p_base, size_t* p_size)
{
    *p_base = (void*)KERNEL_MEM_BASE;
    *p_size = KERNEL_MEM_SIZE;
    return;
}

void hal_mmu_get_usr_addr_range(void** p_base, size_t* p_size)
{
    *p_base = (void*)SANDNIX_KERNEL_PAGE_SIZE;
    *p_size = (address_t)KERNEL_MEM_BASE - (address_t)(*p_base);
    return;
}

kstatus_t hal_mmu_pg_tbl_create(u32 id)
{
    kstatus_t status;

    //Allocate memory
    ppg_tbl_info_t p_pgtbl_info = core_mm_heap_alloc(sizeof(pg_tbl_info_t),
                                  mmu_paging_heap);;

    if(p_pgtbl_info == NULL) {
        status = ENOMEM;
        goto _FAILED_RET;

    }

    if(hal_mmu_phymem_alloc((void**)(&(p_pgtbl_info->physical_addr)), 4096,
                            false, 1)
       == ESUCCESS) {
        status = ENOMEM;
        goto _ALLOC_PDT_FAILED;
    }

    core_pm_spnlck_rw_w_lock(&lock);

    //Clear new pdt
    switch_editing_page((void*)p_pgtbl_info->physical_addr);
    core_rtl_memset(page_operate_addr, 0, 4096);

    core_rtl_map_init(&(p_pgtbl_info->pte_info_map),
                      (item_compare_t)compare_vaddr, mmu_paging_heap);

    //Copy pdt
    ppg_tbl_info_t p_pdt0_info = (ppg_tbl_info_t)core_rtl_array_get(
                                     &mmu_globl_pg_table,
                                     0);
    switch_editing_page((void*)p_pdt0_info->physical_addr);
    static pde_t copy_buf[1024 - KERNEL_MEM_BASE / 4096 / 1024];
    core_rtl_memcpy(copy_buf,
                    (ppde_t)page_operate_addr + KERNEL_MEM_BASE / 4096 / 1024,
                    sizeof(copy_buf));
    switch_editing_page((void*)p_pgtbl_info->physical_addr);
    core_rtl_memcpy((ppde_t)page_operate_addr + KERNEL_MEM_BASE / 4096 / 1024,
                    copy_buf,
                    sizeof(copy_buf));

    //Get index
    if(core_rtl_array_get(&mmu_globl_pg_table, id) != NULL) {
        status = EINVAL;
        goto _NO_FREE_ID;
    }

    if(core_rtl_array_set(&mmu_globl_pg_table, id, p_pgtbl_info) == NULL) {
        status = ENOMEM;
        goto _ADD_TABLE_FAILED;
    }

    core_pm_spnlck_rw_w_unlock(&lock);
    return ESUCCESS;

    //Exception handler
_ADD_TABLE_FAILED:
_NO_FREE_ID:
    core_pm_spnlck_rw_w_unlock(&lock);
_ALLOC_PDT_FAILED:
    core_mm_heap_free(p_pgtbl_info, mmu_paging_heap);
_FAILED_RET:
    return status;
}

void hal_mmu_pg_tbl_destroy(u32 id)
{
    //Release page table id
    core_pm_spnlck_rw_w_lock(&lock);
    ppg_tbl_info_t p_pdt_info = core_rtl_array_get(
                                    &mmu_globl_pg_table,
                                    id);

    if(p_pdt_info == NULL) {
        PANIC(EINVAL,
              "Illegal page table id.");
    }

    core_rtl_array_set(&mmu_globl_pg_table, id, NULL);

    core_pm_spnlck_rw_w_unlock(&lock);

    //Free memory
    hal_mmu_phymem_free((void*)(p_pdt_info->physical_addr));

    core_rtl_map_destroy(&(p_pdt_info->pte_info_map),
                         NULL,
                         (item_destroyer_t)pte_info_destroy,
                         NULL);
    core_mm_heap_free(p_pdt_info, mmu_paging_heap);

    return;
}

kstatus_t hal_mmu_pg_tbl_set(u32 id, void* virt_addr, u32 attribute, void* phy_addr)
{
    core_pm_spnlck_rw_w_lock(&lock);

    //Get page table
    ppg_tbl_info_t p_pdt_info = core_rtl_array_get(&mmu_globl_pg_table,
                                id);

    if(p_pdt_info == NULL) {
        PANIC(EINVAL, "Illegal page table id.");
    }

    if(attribute & MMU_PAGE_AVAIL) {
        //Set page attribute
        kstatus_t status = set_page(p_pdt_info, virt_addr, attribute, phy_addr);
        core_pm_spnlck_rw_w_unlock(&lock);
        refresh_TLB(virt_addr);
        return status;

    } else {
        //Remove page
        remove_page(p_pdt_info, virt_addr);
    }

    core_pm_spnlck_rw_w_unlock(&lock);
    refresh_TLB(virt_addr);
    return ESUCCESS;
}

void hal_mmu_pg_tbl_get(u32 id, void* virt_addr, void** phy_addr, u32* p_attr)
{
    core_pm_spnlck_rw_w_lock(&lock);
    //Get page table
    ppg_tbl_info_t p_pdt_info = core_rtl_array_get(&mmu_globl_pg_table,
                                id);

    if(p_pdt_info == NULL) {
        PANIC(EINVAL, "Illegal page table id.");
    }

    *p_attr = 0;
    ppte_tbl_info_t p_pte_info;

    if((address_t)virt_addr < KERNEL_MEM_BASE) {
        //User page
        p_pte_info = (ppte_tbl_info_t)core_rtl_map_get(
                         &(p_pdt_info->pte_info_map),
                         (void*)((address_t)virt_addr & 0xFFC00000));

    } else {
        //Kernel page
        p_pte_info = (ppte_tbl_info_t)core_rtl_map_get(
                         &mmu_krnl_pte_tbl_map,
                         (void*)((address_t)virt_addr & 0xFFC00000));

    }

    if(p_pte_info == NULL) {
        core_pm_spnlck_rw_w_unlock(&lock);
        return;
    }

    switch_editing_page((void*)(p_pte_info->physical_addr));
    ppte_t p_pte = (ppte_t)page_operate_addr
                   + (address_t)virt_addr % (4096 * 1024) / 4096;

    if(p_pte->present == PG_P) {
        (*p_attr) |= MMU_PAGE_AVAIL;

        if(p_pte->read_write == PG_RW) {
            (*p_attr) |= MMU_PAGE_WRITABLE;
        }

        if(p_pte->cache_disabled == PG_ENCACHE) {
            (*p_attr) |= MMU_PAGE_CACHEABLE;
        }

        (*p_attr) |= MMU_PAGE_EXECUTABLE;
        *phy_addr = (void*)(p_pte->page_base_addr << 12);
    }

    core_pm_spnlck_rw_w_unlock(&lock);
    return;
}

void hal_mmu_pg_tbl_refresh(void* virt_addr)
{
    refresh_TLB(virt_addr);
    return;
}

void hal_mmu_pg_tbl_switch(u32 id)
{
    u32 priority;

    core_pm_spnlck_rw_r_lock(&lock, &priority);
    ppg_tbl_info_t p_pdt_info = (ppg_tbl_info_t)core_rtl_array_get(
                                    &mmu_globl_pg_table,
                                    id);

    if(p_pdt_info == NULL) {
        PANIC(EINVAL,
              "Illegal page table id.");
    }

    __asm__ __volatile__(
        "movl	%0, %%cr3\n"
        ::"a"((p_pdt_info->physical_addr & 0xFFFFF000) | 0x00000008):);

    core_pm_spnlck_rw_r_unlock(&lock, priority);

    return;
}

bool hal_mmu_get_next_mapped_pages(void* start_addr, void** begin, void** p_phy_begin,
                                   size_t* p_size, u32* p_attr)
{
    core_pm_spnlck_rw_w_lock(&lock);

    //Get page table
    *p_attr = 0;
    ppte_tbl_info_t p_pte_info;
    ppte_t p_pte;

    u32 index = (address_t)(start_addr) % (4096 * 1024) / 4096;

    //The first 4KB virtual memory should never be mapped, because of NULL.
    if(core_rtl_map_get(&mmu_krnl_pte_tbl_map,
                        (void*)((address_t)start_addr & 0xFFC00000)) == NULL) {
        *begin = core_rtl_map_next(
                     &mmu_krnl_pte_tbl_map,
                     (void*)((address_t)start_addr & 0xFFC00000));
        index = 0;
    }


    while(true) {
        if(*begin == NULL) {
            core_pm_spnlck_rw_w_unlock(&lock);
            return false;
        }

        p_pte_info = (ppte_tbl_info_t)core_rtl_map_get(
                         &mmu_krnl_pte_tbl_map,
                         (void*)((address_t) * begin & 0xFFC00000));

        if(p_pte_info == NULL) {
            *begin = core_rtl_map_next(
                         &mmu_krnl_pte_tbl_map,
                         (void*)((address_t)(*begin) & 0xFFC00000));
            index = 0;
            continue;
        }

        //Get page info
        switch_editing_page((void*)(p_pte_info->physical_addr));

        //Search page
        for(; index < 1024; index++) {
            p_pte = (ppte_t)page_operate_addr
                    + index;

            if(p_pte->present) {
                break;
            }
        }

        if(index < 1024) {
            //Found page
            *p_phy_begin = (void*)((address_t)(p_pte->page_base_addr) << 12);
            *((address_t*)begin) = ((address_t)(*begin) & 0xFFC00000)
                                   + index * SANDNIX_KERNEL_PAGE_SIZE;
            *p_size = SANDNIX_KERNEL_PAGE_SIZE;
            break;
        }

        //Goto next page table
        *begin = core_rtl_map_next(
                     &mmu_krnl_pte_tbl_map,
                     (void*)((address_t)(*begin) & 0xFFC00000));
        index = 0;
    }

    if(p_pte->present == PG_P) {
        (*p_attr) |= MMU_PAGE_AVAIL;

        if(p_pte->read_write == PG_RW) {
            (*p_attr) |= MMU_PAGE_WRITABLE;
        }

        if(p_pte->cache_disabled == PG_ENCACHE) {
            (*p_attr) |= MMU_PAGE_CACHEABLE;
        }

        (*p_attr) |= MMU_PAGE_EXECUTABLE;
    }

    //Search pages
    address_t base_addr = (address_t)(*begin);

    while(true) {
        base_addr += SANDNIX_KERNEL_PAGE_SIZE;
        p_pte_info = (ppte_tbl_info_t)core_rtl_map_get(
                         &mmu_krnl_pte_tbl_map,
                         (void*)(base_addr & 0xFFC00000));

        if(p_pte_info == NULL) {
            break;
        }

        //Get attribute
        u32 attr = 0;
        switch_editing_page((void*)(p_pte_info->physical_addr));
        ppte_t p_pte = (ppte_t)page_operate_addr
                       + base_addr % (4096 * 1024) / 4096;

        if(p_pte->present == PG_P) {
            attr |= MMU_PAGE_AVAIL;

            if(p_pte->read_write == PG_RW) {
                attr |= MMU_PAGE_WRITABLE;
            }

            if(p_pte->cache_disabled == PG_ENCACHE) {
                attr |= MMU_PAGE_CACHEABLE;
            }

            attr |= MMU_PAGE_EXECUTABLE;
        }

        if(attr != *p_attr) {
            break;
        }

        address_t phy_addr = p_pte->page_base_addr << 12;

        if(base_addr - (address_t)(*begin)
           != phy_addr - (address_t)(*p_phy_begin)) {
            break;
        }

        (*p_size) += SANDNIX_KERNEL_PAGE_SIZE;
    }

    core_pm_spnlck_rw_w_unlock(&lock);
    return true;
}

address_t PRIVATE(get_load_offset)()
{
    return load_offset;
}

void create_0()
{
    ppg_tbl_info_t p_pdt_info;

    //Allocate memory for pdt
    p_pdt_info = core_mm_heap_alloc(sizeof(pg_tbl_info_t), mmu_paging_heap);

    if(p_pdt_info == NULL) {
        PANIC(ENOMEM,
              "Not enough memory for mmu paging managment.");
    }

    //Allocate physical memory
    if(hal_mmu_phymem_alloc((void**)(&(p_pdt_info->physical_addr)), 4096,
                            false, 1) != ESUCCESS) {
        PANIC(ENOMEM,
              "Not enough memory for mmu paging managment.");
    }

    core_rtl_map_init(&(p_pdt_info->pte_info_map),
                      (item_compare_t)compare_vaddr,
                      mmu_paging_heap);

    page_operate_addr = hal_mmu_add_early_paging_addr(
                            (void*)(p_pdt_info->physical_addr), MMU_PAGE_RW);

    if(core_rtl_array_set(&mmu_globl_pg_table, 0, p_pdt_info) == NULL) {
        PANIC(ENOMEM,
              "Not enough memory for mmu paging managment.");
    }

    core_rtl_memset(page_operate_addr, 0, 4096);

    //Copy init_pte_tbl
    for(u32 i = KERNEL_MEM_BASE / 4096 / 1024;
        i <= KERNEL_MEM_BASE / 4096 / 1024 + used_pte_table;
        i++) {
        ppte_tbl_info_t p_pte_info = core_mm_heap_alloc(sizeof(pte_tbl_info_t),
                                     mmu_paging_heap);

        if(p_pte_info == NULL) {
            PANIC(ENOMEM,
                  "Not enough memory for mmu paging managment.");
        }

        p_pte_info->physical_addr = (address_t)(&init_pte_tbl[(i - KERNEL_MEM_BASE / 4096 / 1024)
                                                * 1024]) + load_offset;
        p_pte_info->used_count = 0;
        p_pte_info->freeable = false;

        //Set pde attribute
        switch_editing_page((void*)(p_pdt_info->physical_addr));
        ppde_t p_pde = (ppde_t)page_operate_addr + i;
        p_pde->present = PG_P;
        p_pde->read_write = PG_RW;
        p_pde->user_supervisor = PG_SUPERVISOR;
        p_pde->write_through = PG_WRITE_THROUGH;
        p_pde->cache_disabled = PG_ENCACHE;
        p_pde->page_size = PG_SIZE_4K;
        p_pde->global_page = 1;
        p_pde->page_table_base_addr = (p_pte_info->physical_addr) >> 12;

        //Set pte attribute
        switch_editing_page((void*)(p_pte_info->physical_addr));

        for(int j = 0; j < 1024; j++) {
            ppte_t p_pte = (ppte_t)page_operate_addr + j;
            address_t vaddr = (i * 1024 + j) * 4096;

            if(p_pte->present == PG_P) {
                (p_pte_info->used_count)++;

                if(vaddr >= (address_t)kernel_header.code_start
                   && vaddr < (address_t)kernel_header.code_start + kernel_header.code_size) {
                    //If the page is in code segment
                    p_pte->read_write = PG_RDONLY;

                } else {
                    p_pte->read_write = PG_RW;
                }

                p_pte->user_supervisor = PG_SUPERVISOR;
                p_pte->write_through = PG_WRITE_THROUGH;
                p_pte->cache_disabled = PG_ENCACHE;
                p_pte->global_page = 1;
                hal_early_print_printf("Virtual address %p -> physical address %p.\n",
                                       i * 4096 * 1024 + j * 4096,
                                       p_pte->page_base_addr << 12);

            } else {
                core_rtl_memset(p_pte, 0, sizeof(pte_t));
            }
        }

        if(core_rtl_map_set(&mmu_krnl_pte_tbl_map, (void*)(i * 4096 * 1024),
                            p_pte_info) == NULL) {
            PANIC(ENOMEM,
                  "Not enough memory for mmu paging managment.");
        }
    }

    return;
}

void switch_editing_page(void* phy_addr)
{
    //Get pte
    ppte_t p_pte = (ppte_t)((init_pde_tbl[(address_t)page_operate_addr / (4096 * 1024)]
                             .page_table_base_addr << 12) - load_offset);
    p_pte += (address_t)page_operate_addr % (4096 * 1024) / 4096;
    p_pte->page_base_addr = (address_t)phy_addr >> 12;

    //Invalidate TLB
    refresh_TLB((void*)page_operate_addr);

    return;
}

void refresh_TLB(void* address)
{
    __asm__ __volatile__(
        "invlpg		(%0)\n"
        ::"a"(address));
    return;
}

int compare_vaddr(address_t p1, address_t p2)
{
    if(p1 > p2) {
        return 1;

    } else if(p1 == p2) {
        return 0;

    } else {
        return -1;
    }
}

void remove_page(ppg_tbl_info_t p_pdt_info, void* virt_addr)
{
    ppte_tbl_info_t p_pte_info;

    if((address_t)virt_addr >= KERNEL_MEM_BASE) {
        //Kernel page
        p_pte_info = (ppte_tbl_info_t)core_rtl_map_get(
                         &mmu_krnl_pte_tbl_map,
                         (void*)((address_t)virt_addr & 0xFFC00000));

    } else {
        //User page
        p_pte_info = (ppte_tbl_info_t)core_rtl_map_get(
                         &(p_pdt_info->pte_info_map),
                         (void*)((address_t)virt_addr & 0xFFC00000));
    }

    if(p_pte_info == NULL) {
        return;
    }

    switch_editing_page((void*)(p_pte_info->physical_addr));
    //Set pte attribute
    ppte_t p_pte = (ppte_t)page_operate_addr;
    p_pte += (address_t)virt_addr % (4096 * 1024) / 4096;

    if(p_pte->avail == PG_NP) {
        return;
    }

    p_pte->avail = PG_NP;
    (p_pte_info->used_count)--;

    if(p_pte_info->used_count == 0 && p_pte_info->freeable) {
        //Remove pte table
        if((address_t)virt_addr >= KERNEL_MEM_BASE) {
            core_rtl_map_set(&mmu_krnl_pte_tbl_map,
                             (void*)((address_t)virt_addr & 0xFFC00000),
                             NULL);

        } else {
            core_rtl_map_set(&(p_pdt_info->pte_info_map),
                             (void*)((address_t)virt_addr & 0xFFC00000),
                             NULL);

        }

        hal_mmu_phymem_free((void*)(p_pte_info->physical_addr));
        core_mm_heap_free(p_pte_info, mmu_paging_heap);

        //Unmap pde
        switch_editing_page((void*)(p_pdt_info->physical_addr));
        ppde_t p_pde = (ppde_t)page_operate_addr + (address_t)virt_addr / 1024 / 4096;
        p_pde->present = PG_NP;

        if((address_t)virt_addr >= KERNEL_MEM_BASE) {
            krnl_pdt_sync(p_pdt_info);
        }
    }

    return;
}

kstatus_t set_page(ppg_tbl_info_t p_pdt_info, void* virt_addr, u32 attr,
                   void* phy_addr)
{
    ppte_tbl_info_t p_pte_info;

    if((address_t)virt_addr >= KERNEL_MEM_BASE) {
        //Kernel page
        p_pte_info = (ppte_tbl_info_t)core_rtl_map_get(
                         &mmu_krnl_pte_tbl_map,
                         (void*)((address_t)virt_addr & 0xFFC00000));

    } else {
        //User page
        p_pte_info = (ppte_tbl_info_t)core_rtl_map_get(
                         &(p_pdt_info->pte_info_map),
                         (void*)((address_t)virt_addr & 0xFFC00000));
    }

    if(p_pte_info == NULL) {
        //Create new pte table
        p_pte_info = core_mm_heap_alloc(sizeof(pte_tbl_info_t),
                                        mmu_paging_heap);

        if(p_pte_info == NULL) {
            return ENOMEM;
        }

        p_pte_info->freeable = true;
        kstatus_t status = hal_mmu_phymem_alloc((void**)(&(p_pte_info->physical_addr)),
                                                4096,
                                                false, 1);

        if(status != ESUCCESS) {
            core_mm_heap_free(p_pte_info, mmu_paging_heap);
            return status;
        }

        switch_editing_page((void*)(p_pte_info->physical_addr));
        core_rtl_memset(page_operate_addr, 0, 4096);
        p_pte_info->used_count = 0;

        //Add to pte info map
        if((address_t)virt_addr >= KERNEL_MEM_BASE) {
            //Kernel page
            if(core_rtl_map_set(&mmu_krnl_pte_tbl_map,
                                (void*)((address_t)virt_addr & 0xFFC00000),
                                p_pte_info) == NULL) {
                hal_mmu_phymem_free((void*)(p_pte_info->physical_addr));
                core_mm_heap_free(p_pte_info, mmu_paging_heap);
                return ENOMEM;
            }

        } else {
            //User page
            if(core_rtl_map_set(&(p_pdt_info->pte_info_map),
                                (void*)((address_t)virt_addr & 0xFFC00000),
                                p_pte_info) == NULL) {
                hal_mmu_phymem_free((void*)(p_pte_info->physical_addr));
                core_mm_heap_free(p_pte_info, mmu_paging_heap);
                return ENOMEM;
            }
        }

        //Add mapping
        switch_editing_page((void*)(p_pdt_info->physical_addr));
        ppde_t p_pde = (ppde_t)page_operate_addr
                       + (address_t)virt_addr / 4096 / 1024;

        p_pde->present = PG_P;
        p_pde->read_write = PG_RW;

        if((address_t)virt_addr >= KERNEL_MEM_BASE) {
            p_pde->user_supervisor = PG_SUPERVISOR;
            p_pde->global_page = 1;

        } else {
            p_pde->user_supervisor = PG_USER;
            p_pde->global_page = 0;
        }

        p_pde->write_through = PG_WRITE_THROUGH;
        p_pde->cache_disabled = PG_ENCACHE;
        p_pde->page_size = PG_SIZE_4K;
        p_pde->page_table_base_addr = (p_pte_info->physical_addr) >> 12;

        if((address_t)virt_addr >= KERNEL_MEM_BASE) {
            krnl_pdt_sync(p_pdt_info);
        }
    }

    //Set pte attribute
    switch_editing_page((void*)(p_pte_info->physical_addr));
    ppte_t p_pte = (ppte_t)page_operate_addr
                   + (address_t)virt_addr % (4096 * 1024) / 4096;

    if(p_pte->avail == PG_NP) {
        (p_pte_info->used_count)++;
    }

    p_pte->present = PG_P;

    if(attr & MMU_PAGE_WRITABLE) {
        p_pte->read_write = PG_RW;

    } else {
        p_pte->read_write = PG_RDONLY;
    }

    if(attr & MMU_PAGE_CACHEABLE) {
        p_pte->cache_disabled = PG_ENCACHE;

    } else {
        p_pte->cache_disabled = PG_DISCACHE;
    }

    if((address_t)virt_addr > KERNEL_MEM_BASE) {
        p_pte->user_supervisor = PG_SUPERVISOR;
        p_pte->global_page = 1;

    } else {
        p_pte->user_supervisor = PG_USER;
        p_pte->global_page = 0;
    }

    p_pte->write_through = PG_WRITE_THROUGH;
    p_pte->accessed = 0;
    p_pte->dirty = 0;
    p_pte->page_table_attr_index = 0;
    p_pte->avail = 0;
    p_pte->page_base_addr = (address_t)phy_addr >> 12;

    return ESUCCESS;
}

void krnl_pdt_sync(ppg_tbl_info_t p_pdt_info)
{
    static pde_t sync_buf[(0 - KERNEL_MEM_BASE) / (1024 * 4096)];
    switch_editing_page((void*)(p_pdt_info->physical_addr));

    //Copy page table
    core_rtl_memcpy(sync_buf,
                    (ppde_t)page_operate_addr + KERNEL_MEM_BASE / (1024 * 4096),
                    (0 - KERNEL_MEM_BASE) / (1024 * 4096));

    u32 index = 0;

    while(core_rtl_array_get_used_index(&mmu_globl_pg_table,
                                        index,
                                        &index)) {
        ppg_tbl_info_t p_pdt_info = (ppg_tbl_info_t)core_rtl_array_get(
                                        &mmu_globl_pg_table,
                                        index);
        switch_editing_page((void*)(p_pdt_info->physical_addr));
        core_rtl_memcpy((ppde_t)page_operate_addr + KERNEL_MEM_BASE / (1024 * 4096),
                        sync_buf,
                        (0 - KERNEL_MEM_BASE) / (1024 * 4096));
        index++;
    }

    return;
}

void pte_info_destroy(ppte_tbl_info_t p_pte_info, void* p_null)
{
    if(p_pte_info->freeable) {
        hal_mmu_phymem_free((void*)(p_pte_info->physical_addr));
    }

    core_mm_heap_free(p_pte_info, mmu_paging_heap);
    UNREFERRED_PARAMETER(p_null);
    return;
}
