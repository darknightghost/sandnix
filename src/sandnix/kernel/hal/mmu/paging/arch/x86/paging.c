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
#include "../../../../init/init.h"


#define	REQUIRED_INIT_PAGE_NUM	((1024 * 1024 + KERNEL_MAX_SIZE) / 4096)
#define	MAX_INIT_PAGE_NUM		(REQUIRED_INIT_PAGE_NUM % 1024 > 0 \
                                 ? (REQUIRED_INIT_PAGE_NUM / 1024 + 1) * 1024 \
                                 : REQUIRED_INIT_PAGE_NUM)


static pde_t __attribute__((aligned(4096)))	init_pde_tbl[1024];
static pte_t __attribute__((aligned(4096)))	init_pte_tbl[MAX_INIT_PAGE_NUM];
static u32		used_pte_table;


static bool		initialized = false;


void start_paging()
{
    address_t offset;
    ppde_t p_init_pde_tbl;
    ppde_t p_pde;
    ppte_t p_init_pte_tbl;
    ppte_t p_pte;
    pkrnl_hdr_t p_krnl_header;
    size_t total_pg_num;
    size_t total_map_size;
    u32 pde_num;
    u32 empty_pte_num;

    if(initialized) {
        //TODO:panic
    }

    /*
     * When this function is called, the paging has not been started.We need to
     * compute the offset in order to get the physical address of global
     * variables.
     */
    //Compute the offset.
    __asm__ __volatile__(
        "call	_ADDR\n"
        "_ADDR:\n"
        "popl	%%eax\n"
        "subl	$_ADDR,%%eax\n"
        "movl	%%eax, %0\n"
        ::"m"(offset)
        :"eax", "memory");

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

    return;
}

void* hal_mmu_add_early_paging_addr(void* phy_addr)
{
    ppde_t p_pde;
    ppte_t p_pte;
    u32 i;
    u32 index;

    if(initialized) {
        //TODO: panic
    }

    //Test if the PDE item presents
    p_pde = init_pde_tbl + (((address_t)phy_addr + KERNEL_MEM_BASE)
                            >> 22);	//4096 * 1024 == 2^22

    if(p_pde->present == PG_P) {
        //The PDE item exists.
        p_pte = (ppte_t)(((p_pde->page_table_base_addr) << 12) + KERNEL_MEM_BASE);
        index = ((address_t)phy_addr) % (4096 * 1024) / 4096;
        p_pte += index;

        if(p_pte->present == PG_P) {
            return (void*)((address_t)phy_addr + KERNEL_MEM_BASE);
        }

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
        p_pte->page_base_addr = ((address_t)phy_addr) >> 12;

    } else {
        //The PDE item does not exists.
        //Get a new PTE table
        if(used_pte_table >= MAX_INIT_PAGE_NUM) {
            //TODO:panic
        }

        used_pte_table++;
        p_pte = init_pte_tbl + 1024 * used_pte_table;

        //Initialize pde node
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
        p_pde->page_table_base_addr = ((address_t)p_pte) >> 12;

        //Initialize pte table
        index = ((address_t)phy_addr) % (4096 * 1024) / 4096;

        for(i = 0; i < 1024; i++) {
            if(i == index) {
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
                p_pte->page_base_addr = ((address_t)phy_addr) >> 12;

            } else {
                p_pte->present = PG_NP;
            }

            p_pte++;
        }

    }

    return (void*)((address_t)phy_addr + KERNEL_MEM_BASE);
}
