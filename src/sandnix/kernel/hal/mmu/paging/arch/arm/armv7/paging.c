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

#include "../../../paging.h"
#include "../../../../mmu.h"
#include "../../../../../exception/exception.h"
#include "../../../../../init/init.h"
#include "../../../../../early_print/early_print.h"
#include "../../../../../debug/debug.h"

/*
 *   Though ARM supports different size of pages, we only use 4KB pages in order
 * to make our kernel compatibal with different architectures of CPUs.
 */

/*
 *   On this condition, lv1 table has 16KB / 4 bytes = 4096 entries,
 * lv2 table has 1KB / 4 bytes = 256 entries.
 * Total memory is 256 * 4096 *4096 bytes = 4GB
 */
#if	KERNEL_MEM_BASE % (4096 * 256) !=0
    #error	"KERNEL_MEM_BASE must be 1MB aligned."
#endif

#define	REQUIRED_INIT_PAGE_NUM	((KERNEL_MAX_SIZE) / 4096 + 1)
#define	MAX_INIT_PAGE_NUM		(REQUIRED_INIT_PAGE_NUM % (256 * 4) > 0 \
                                 ? (REQUIRED_INIT_PAGE_NUM / (256 * 4) + 1) \
                                 * (256 * 4)\
                                 : REQUIRED_INIT_PAGE_NUM)

static lv1_pg_desc_t __attribute__((aligned(1024 * 16)))	init_lv1_pg_tbl[4096];
static lv2_pg_desc_t __attribute__((aligned(1024)))			init_lv2_pg_tbl[MAX_INIT_PAGE_NUM];
static u32			used_lv2_desc;
static address_t	load_offset;
static array_t		mmu_globl_pg_table;
static map_t		mmu_krnl_lv2_tbl_map;
static pheap_t		mmu_paging_heap;
static u8			mmu_paging_heap_block[4096];
static void*		page_operate_addr;
static spnlck_rw_t	lock;

static bool		initialized = false;

static inline	void	lv1_prepare(address_t kernel_base, size_t kernel_size,
                                    size_t offset);
static inline	void	lv2_prepare(address_t kernel_base, size_t kernel_size,
                                    size_t offset);

static inline void	init_SCTLR();
static inline void	init_TTBCR();
static inline void	enable_mmu();
static inline void	enable_dcache();
static inline void	enable_icache();
static inline void	disable_dcache();
static inline void	disable_icache();
static inline void	invalidate_TLBs();
static inline void	invalidate_TLB(void* virt_addr);
static inline void	invalidate_icache();
static inline void	invalidate_dcache();
static inline void	load_TTBR(address_t phyaddr);

static int			compare_vaddr(address_t p1, address_t p2);
static void			create_0();
static void			switch_editing_page(void* phy_addr);
static void			lv2_create0(u32 used_lv2_tabel);
static void			remove_page(plv1_tbl_info_t p_lv1_info, void* virt_addr);
static kstatus_t	set_page(plv1_tbl_info_t p_lv1_info, void* virt_addr,
                             u32 attr, void* phy_addr);
static void			krnl_pdt_sync(plv1_tbl_info_t p_lv1_info);
static void			lv2_info_destroy(plv2_tbl_info_t p_lv2_info, void* p_null);

void start_paging()
{
    //Compute offset
    address_t offset;
    __asm__ __volatile__(
        "_ADDR1:\n"
        "mov	%0, pc\n"
        "sub	%0, %0, #8\n"
        "ldr	r0, =_ADDR1\n"
        "sub	%0, %0, r0\n"
        "b		1f\n"
        ".ltorg\n"
        "1:\n"
        :"=r"(offset)
        ::"r0");

    if(offset == 0) {
        PANIC(ENOTSUP,
              "Function start_paging() can only "
              "be used when mmu module has not been initialized");
    }

    //Get kernel base and kernel address
    pkrnl_hdr_t p_kkeader = (pkrnl_hdr_t)((address_t)(&kernel_header) + offset);
    address_t kernel_base = MIN((address_t)(p_kkeader->code_start),
                                (address_t)(p_kkeader->data_start));
    address_t kernel_end = MAX((address_t)(p_kkeader->code_start) + p_kkeader->code_size,
                               (address_t)(p_kkeader->data_start) + p_kkeader->data_size);

    kernel_base = ((kernel_base % 4096)
                   ? kernel_base & (~(4096 - 1))
                   : kernel_base);
    size_t kernel_size = kernel_end - kernel_base;
    kernel_size = ((kernel_size % 4096)
                   ? (kernel_size & (~(4096 - 1))) + 4096
                   : kernel_size);

    //Prepare first-level page table.
    lv1_prepare(kernel_base, kernel_size, offset);

    //Prepare second-level page table.
    lv2_prepare(kernel_base, kernel_size, offset);

    //Initialize registers
    init_SCTLR();
    init_TTBCR();

    //Load page table
    load_TTBR((address_t)init_lv1_pg_tbl + offset);
    invalidate_TLBs();

    //Start paging
    invalidate_icache();
    enable_mmu();
    enable_dcache();
    enable_icache();

    load_offset = offset;
    return;
}

void* hal_mmu_add_early_paging_addr(void* phy_addr, u32 attr)
{
    if(initialized) {
        PANIC(ENOTSUP,
              "Function hal_mmu_add_early_paging_addr() can only "
              "be used when mmu module has not been initialized");
    }

    if(!(attr & MMU_PAGE_AVAIL)) {
        PANIC(EINVAL,
              "Cannot add an unavailable page.");
    }

    address_t pg_addr = (address_t)phy_addr;
    pg_addr -= pg_addr % 4096;
    address_t addr_off = (address_t)phy_addr - pg_addr;

    //Check if the address has been mapped
    u32 i;

    for(i = 0; i < used_lv2_desc; i++) {
        if(LV2_DESC_TYPE(&init_lv2_pg_tbl[i]) == MMU_PG_4KB) {
            if(LV2_4KB_GET_ADDR(&init_lv2_pg_tbl[i]) == pg_addr) {
                //The address has been mapped
                return (void*)(KERNEL_MEM_BASE + i * 4096 + addr_off);
            }
        }
    }

    //Get a free page
    if(used_lv2_desc >= MAX_INIT_PAGE_NUM) {
        PANIC(ENOMEM, "Not enough temporary page table!\n");
    }

    used_lv2_desc++;

    //Fill the page descriptor
    LV2_DESC_TYPE_SET(&init_lv2_pg_tbl[i], MMU_PG_4KB);

    if(attr & MMU_PAGE_EXECUTABLE) {
        init_lv2_pg_tbl[i].pg_4KB.xn = 0;

    } else {
        init_lv2_pg_tbl[i].pg_4KB.xn = 1;
    }

    init_lv2_pg_tbl[i].pg_4KB.reserv1 = 0;
    init_lv2_pg_tbl[i].pg_4KB.reserv2 = 0;
    init_lv2_pg_tbl[i].pg_4KB.reserv3 = 0;

    if(attr & MMU_PAGE_WRITABLE) {
        LV2_SET_AP(&init_lv2_pg_tbl[i], PG_AP_RW_NA);

    } else {
        LV2_SET_AP(&init_lv2_pg_tbl[i], PG_AP_RO_NA);
    }

    LV2_4KB_SET_ADDR(&init_lv2_pg_tbl[i], pg_addr);

    if(i % 256 == 0) {
        //Fill the lv1 descriptor
        plv1_pg_desc_t p_lv1_desc = &init_lv1_pg_tbl[i / 256
                                    + KERNEL_MEM_BASE / 4096 / 256];
        LV1_DESC_TYPE_SET(p_lv1_desc, MMU_PG_LV2ENT);
        p_lv1_desc->lv2_entry.reserv1 = 0;
        p_lv1_desc->lv2_entry.none_secure = 1;
        p_lv1_desc->lv2_entry.reserv2 = 0;
        LV1_LV2ENT_SET_ADDR(p_lv1_desc, (address_t)(&init_lv2_pg_tbl[i])
                            + load_offset);
    }

    void* ret = (void*)(KERNEL_MEM_BASE + i * 4096 + addr_off);
    invalidate_TLB(ret);

    return ret;
}

void paging_init()
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

    core_rtl_map_init(&mmu_krnl_lv2_tbl_map, (item_compare_t)(compare_vaddr),
                      mmu_paging_heap);

    //Create process 0 page table
    hal_early_print_printf("Creating paging table 0...\n");
    create_0();

    //Initialize lock
    core_pm_spnlck_rw_init(&lock);

    //Switch to page 0
    hal_early_print_printf("Switching to page table 0...\n");
    hal_mmu_pg_tbl_switch(0);
    invalidate_TLBs();
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

kstatus_t hal_mmu_pg_tbl_create(u32* p_id)
{
    kstatus_t status;

    //Allocate lv1 table
    plv1_tbl_info_t p_lv1_info = core_mm_heap_alloc(sizeof(lv1_tbl_info_t),
                                 mmu_paging_heap);

    if(p_lv1_info == NULL) {
        status = ENOMEM;
        goto _ALLOC_FAILED;
    }

    if(hal_mmu_phymem_alloc((void**)(&(p_lv1_info->physical_addr)),
                            16 * 1024, false, 4) != ESUCCESS) {
        status = ENOMEM;
        goto _ALLOC_PHYMEM_FAILED;
    }

    //Clear new lv1 table
    core_pm_spnlck_rw_w_lock(&lock);
    switch_editing_page((void*)(p_lv1_info->physical_addr));
    core_rtl_memset((void*)page_operate_addr, 0, 4096);
    core_rtl_map_init(&(p_lv1_info->lv2_info_map),
                      (item_compare_t)compare_vaddr, mmu_paging_heap);

    //Copy kernel lv1 table
    static lv1_pg_desc_t copy_buf[1024];
    plv1_tbl_info_t p_lv1_0 = core_rtl_array_get(&mmu_globl_pg_table, 0);

    for(u32 i = 0; i < 4; i++) {
        if(i >= KERNEL_MEM_BASE / 4096 / 256 / 1024) {
            switch_editing_page((void*)(p_lv1_0->physical_addr + i * 4096));

            if(i * 1024 < KERNEL_MEM_BASE / 4096 / 256) {
                void* base_addr = (void*)((KERNEL_MEM_BASE / 4096 / 256 - i * 1024)
                                          * sizeof(lv1_pg_desc_t) + (address_t)page_operate_addr);
                size_t sz = (address_t)page_operate_addr + 4096
                            - (address_t)base_addr;
                core_rtl_memcpy(copy_buf, base_addr, sz);
                switch_editing_page((void*)(p_lv1_info->physical_addr + i * 4096));
                core_rtl_memcpy(base_addr, copy_buf, sz);

            } else {
                core_rtl_memcpy(copy_buf, (void*)page_operate_addr,
                                sizeof(copy_buf));
                switch_editing_page((void*)(p_lv1_info->physical_addr + i * 4096));
                core_rtl_memcpy((void*)page_operate_addr, copy_buf,
                                sizeof(copy_buf));
            }
        }
    }

    //Get index
    if(!core_rtl_array_get_free_index(&mmu_globl_pg_table, p_id)) {
        status = ENOMEM;
        goto _NO_FREE_ID;
    }

    if(core_rtl_array_set(&mmu_globl_pg_table, *p_id, p_lv1_info) == NULL) {
        status = ENOMEM;
        goto _ADD_TABLE_FAILED;
    }

    core_pm_spnlck_rw_w_unlock(&lock);
    return ESUCCESS;

    //Exception handler
_ADD_TABLE_FAILED:
_NO_FREE_ID:
    core_pm_spnlck_rw_w_unlock(&lock);
_ALLOC_PHYMEM_FAILED:
    core_mm_heap_free(p_lv1_info, mmu_paging_heap);
_ALLOC_FAILED:
    return status;

}

void hal_mmu_pg_tbl_destroy(u32 id)
{
    //Release page table id
    core_pm_spnlck_rw_w_lock(&lock);
    plv1_tbl_info_t p_lv1_info = core_rtl_array_get(
                                     &mmu_globl_pg_table,
                                     id);

    if(p_lv1_info == NULL) {
        PANIC(EINVAL,
              "Illegal page table id.");
    }

    core_rtl_array_set(&mmu_globl_pg_table, id, NULL);

    core_pm_spnlck_rw_w_unlock(&lock);

    //Free memory
    hal_mmu_phymem_free((void*)(p_lv1_info->physical_addr));

    core_rtl_map_destroy(&(p_lv1_info->lv2_info_map),
                         NULL,
                         (item_destroyer_t)lv2_info_destroy,
                         NULL);
    core_mm_heap_free(p_lv1_info, mmu_paging_heap);

    return;
}

kstatus_t hal_mmu_pg_tbl_set(u32 id, void* virt_addr, u32 attribute,
                             void* phy_addr)
{
    core_pm_spnlck_rw_w_lock(&lock);

    //Get page table
    plv1_tbl_info_t p_lv1_info = core_rtl_array_get(&mmu_globl_pg_table,
                                 id);

    if(p_lv1_info == NULL) {
        PANIC(EINVAL, "Illegal page table id.");
    }

    if(attribute & MMU_PAGE_AVAIL) {
        //Set page attribute
        kstatus_t status = set_page(p_lv1_info, virt_addr, attribute, phy_addr);
        core_pm_spnlck_rw_w_unlock(&lock);
        invalidate_TLB(virt_addr);
        return status;

    } else {
        //Remove page
        remove_page(p_lv1_info, virt_addr);
    }

    core_pm_spnlck_rw_w_unlock(&lock);
    invalidate_TLB(virt_addr);
    return ESUCCESS;
}

void hal_mmu_pg_tbl_get(u32 id, void* virt_addr, void** phy_addr, u32* p_attr)
{
    plv2_tbl_info_t p_lv2_info;

    core_pm_spnlck_rw_w_lock(&lock);
    plv1_tbl_info_t p_lv1_info = (plv1_tbl_info_t)core_rtl_array_get(
                                     &mmu_globl_pg_table,
                                     id);

    if(p_lv1_info == NULL) {
        PANIC(EINVAL,
              "Illegal page table id.");
    }

    if((address_t)virt_addr >= KERNEL_MEM_BASE) {
        //Kernel page
        p_lv2_info = (plv2_tbl_info_t)core_rtl_map_get(
                         &mmu_krnl_lv2_tbl_map,
                         (void*)((address_t)virt_addr & 0xFFC00000));

    } else {
        //User page
        p_lv2_info = (plv2_tbl_info_t)core_rtl_map_get(
                         &(p_lv1_info->lv2_info_map),
                         (void*)((address_t)virt_addr & 0xFFC00000));

    }

    if(p_lv2_info == NULL) {
        //The page did not be mapped.
        *phy_addr = NULL;
        *p_attr = MMU_PAGE_UNAVAIL;
        goto _END;
    }

    switch_editing_page((void*)(p_lv2_info->physical_addr));
    plv2_pg_desc_t p_lv2_desc = (plv2_pg_desc_t)page_operate_addr
                                + (address_t)virt_addr % (256 * 4 * 4096) / 4096;

    *p_attr = 0;

    if(LV2_DESC_TYPE(p_lv2_desc) == MMU_PG_FAULT) {
        //The page did not be mapped.
        *phy_addr = NULL;
        goto _END;

    }

    *p_attr |= MMU_PAGE_AVAIL;

    if(p_lv2_desc->pg_4KB.xn == 0) {
        *p_attr |= MMU_PAGE_EXECUTABLE;
    }

    switch(LV2_GET_AP(p_lv2_desc)) {
        case PG_AP_RW_NA:
        case PG_AP_RW_RO:
        case PG_AP_RW_RW:
            *p_attr |= MMU_PAGE_WRITABLE;
    }

    *phy_addr = (void*)LV2_4KB_GET_ADDR(p_lv2_desc);

_END:
    core_pm_spnlck_rw_w_unlock(&lock);

    return;
}

void hal_mmu_pg_tbl_refresh(void* virt_addr)
{
    invalidate_TLB(virt_addr);
    return;
}

void hal_mmu_pg_tbl_switch(u32 id)
{
    core_pm_spnlck_rw_r_lock(&lock);
    plv1_tbl_info_t p_lv1_info = (plv1_tbl_info_t)core_rtl_array_get(
                                     &mmu_globl_pg_table,
                                     id);

    if(p_lv1_info == NULL) {
        PANIC(EINVAL,
              "Illegal page table id.");
    }

    load_TTBR(p_lv1_info->physical_addr);
    core_pm_spnlck_rw_r_unlock(&lock);
    invalidate_TLBs();

    return;
}

address_t get_load_offset()
{
    return load_offset;
}

void lv1_prepare(address_t kernel_base, size_t kernel_size, size_t offset)
{
    //Compute how may entries to fill
    u32 page_num = kernel_size / 4096;
    u32 lv1_entry_num = page_num / 256;

    if(page_num % 256 != 0) {
        lv1_entry_num++;
    }

    //Prepare lv1 table
    plv1_pg_desc_t p_lv1_desc = (plv1_pg_desc_t)((address_t)init_lv1_pg_tbl
                                + offset);
    address_t kernel_phy_base = kernel_base + offset;

    if(kernel_size % (256 * 4096) > 0) {
        kernel_size = (kernel_size / (256 * 4096) + 1) * 256 * 4096;
    }

    for(u32 i = 0;
        i < 4096;
        i++, p_lv1_desc++) {
        if(i >= kernel_phy_base / (256 * 4096)
           && i <= (kernel_phy_base + kernel_size) / (256 * 4096)) {
            //Current address
            LV1_DESC_TYPE_SET(p_lv1_desc, MMU_PG_LV2ENT);
            p_lv1_desc->lv2_entry.reserv1 = 0;
            p_lv1_desc->lv2_entry.none_secure = 1;
            p_lv1_desc->lv2_entry.reserv2 = 0;
            LV1_LV2ENT_SET_ADDR(p_lv1_desc,
                                (i - kernel_phy_base / (256 * 4096)) * 256 * sizeof(lv2_pg_desc_t)
                                + (address_t)init_lv2_pg_tbl + offset);


        } else if(i >= kernel_base / (256 * 4096)
                  && i <= (kernel_base + kernel_size) / (256 * 4096)) {
            //Kernel memoy
            LV1_DESC_TYPE_SET(p_lv1_desc, MMU_PG_LV2ENT);
            p_lv1_desc->lv2_entry.reserv1 = 0;
            p_lv1_desc->lv2_entry.none_secure = 1;
            p_lv1_desc->lv2_entry.reserv2 = 0;
            LV1_LV2ENT_SET_ADDR(p_lv1_desc,
                                (i - kernel_base / (256 * 4096)) * 256 * sizeof(lv2_pg_desc_t)
                                + (address_t)init_lv2_pg_tbl + offset);

        } else {
            LV1_DESC_TYPE_SET(p_lv1_desc, MMU_PG_FAULT);
        }
    }

    return;
}

void lv2_prepare(address_t kernel_base, size_t kernel_size, size_t offset)
{
    //Compute number of pages to fill
    u32 pg_num = kernel_size / 4096;
    u32 fill_num = pg_num;

    if(fill_num % (256 * 4) != 0) {
        fill_num = fill_num + (256 * 4 - fill_num % (256 * 4));
    }

    //Prepare lv2 page table
    plv2_pg_desc_t p_lv2_desc
        = (plv2_pg_desc_t)((address_t)init_lv2_pg_tbl + offset);
    address_t kernel_phy_base = kernel_base + offset;

    for(u32 i = 0;
        i < fill_num;
        i++, p_lv2_desc++) {
        if(i < pg_num) {
            LV2_DESC_TYPE_SET(p_lv2_desc, MMU_PG_4KB);
            p_lv2_desc->pg_4KB.xn = 0;
            p_lv2_desc->pg_4KB.reserv1 = 0;
            p_lv2_desc->pg_4KB.reserv2 = 0;
            p_lv2_desc->pg_4KB.reserv3 = 0;
            LV2_SET_AP(p_lv2_desc, PG_AP_RW_NA);
            LV2_4KB_SET_ADDR(p_lv2_desc, kernel_phy_base + i * 4096);

        } else {
            LV2_DESC_TYPE_SET(p_lv2_desc, MMU_PG_FAULT);
        }
    }

    *((u32*)((address_t)(&used_lv2_desc) + offset)) = pg_num;
    return;
}

void init_SCTLR()
{
#define	MASK	0x62007405
#define	VAL		0x50010820
    /*
     * Theses bits of SCTLR is setted to:
     *		Bit 0(M)		: 0
     *		Bit 2(C)		: 0
     *		Bit 10(SW)		: 1
     *		Bit	12(I)		: 0
     *		Bit 13(V)		: 0
     *		Bit 14(RR)		: 1
     *		Bit	25(EE)		: 0
     *		Bit	29(AFE)		: 0
     *		Bit 30(TE)		: 0
     */
    __asm__ __volatile__(
        "mrc    p15, 0, r0, c1, c0, 0\n"
        "and	r0,	r0,	%0\n"
        "orr	r0, r0, %1\n"
        "mcr    p15, 0, r0, c1, c0, 0\n"
        ::"r"(~MASK), "r"(VAL)
        :"r0");
#undef	VAL
#undef	MASK
    return;
}

void init_TTBCR()
{
#define	MASK	0x80000037
    /*
     * Theses bits of TTBCR is setted to:
     *		Bit	2:0(N)		:0
     *		Bit	4(PD0)		:0
     *		Bit	5(PD1)		:0
     *		Bit	31(EAE)		:0
     */
    __asm__ __volatile__(
        "mrc    p15, 0, r0, c2, c0, 2\n"
        "and	r0,	r0,	%0\n"
        "mcr    p15, 0, r0, c2, c0, 2\n"
        ::"r"(~MASK)
        :"r0");
#undef	MASK
    return;

}

void enable_mmu()
{
    __asm__ __volatile__(
        "mrc    p15, 0, r0, c1, c0, 0\n"
        "orr	r0, r0, #3\n"
        "mcr    p15, 0, r0, c1, c0, 0\n"
        :::"r0");

    return;
}

void load_TTBR(address_t phyaddr)
{
    address_t val = (phyaddr & 0xFFFFC000) | 0x00000011;
    __asm__ __volatile__(
        "mcr    p15, 0, %0, c2, c0, 0\n"
        "nop\n"
        "nop\n"
        ::"r"(val)
        :);
    return;
}

void invalidate_TLBs()
{
    __asm__  __volatile__(
        "mov    r0, #0\n"
        "mcr    p15, 0, r0, c8, c7, 0\n"
        :::"r0");
    return;
}

void invalidate_TLB(void* virt_addr)
{
    __asm__ __volatile__(
        "mcr    p15, 0, %0, c8, c7, 1\n"
        ::"r"((address_t)virt_addr & 0xFFFFF000)
        :"memory");
}

void invalidate_icache()
{
    __asm__  __volatile__(
        "mov    r0, #0\n"
        "mcr    p15, 0, r0, c7, c5, 0\n"
        "nop\n"
        "nop\n"
        ::: "r0");
}

void invalidate_dcache()
{
    __asm__  __volatile__(
        "mov    r0,#0\n"
        "mcr    p15,0,r0,c7,c6,0\n"
        "nop\n"
        "nop\n"
        ::: "r0");
}

void enable_dcache()
{
    __asm__  __volatile__(
        "mrc    p15, 0, r0, c1, c0, 0\n"
        "orr    r0, r0, #4\n"
        "mcr    p15, 0, r0, c1, c0, 0\n"
        ::: "r0");
}

void enable_icache()
{
    __asm__  __volatile__(
        "mrc    p15, 0, r0, c1, c0, 0\n"
        "orr    r0, r0, #0x1000\n"
        "mcr    p15, 0, r0, c1, c0, 0\n"
        ::: "r0");
}

void disable_dcache()
{
    __asm__  __volatile__(
        "mrc    p15, 0, r0, c1, c0, 0\n"
        "and    r0, r0, %0\n"
        "mcr    p15, 0, r0, c1, c0, 0\n"
        ::"r"(~(u32)(0x04))
        : "r0");
}

void disable_icache()
{
    __asm__  __volatile__(
        "mrc    p15, 0, r0, c1, c0, 0\n"
        "and    r0, r0, %0\n"
        "mcr    p15, 0, r0, c1, c0, 0\n"
        ::"r"(~(u32)(0x1000))
        : "r0");
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

void create_0()
{
    //Allocate memory for lv1 table
    plv1_tbl_info_t p_lv1_info = core_mm_heap_alloc(sizeof(lv1_tbl_info_t),
                                 mmu_paging_heap);

    if(p_lv1_info == NULL) {
        PANIC(ENOMEM,
              "Not enough memory for mmu paging managment.");
    }

    //Allocate physical memory
    if(hal_mmu_phymem_alloc((void**)(&(p_lv1_info->physical_addr)),
                            1024 * 16,
                            false, 4) != ESUCCESS) {
        PANIC(ENOMEM,
              "Not enough memory for mmu paging managment.");
    }

    core_rtl_map_init(&(p_lv1_info->lv2_info_map),
                      (item_compare_t)compare_vaddr,
                      mmu_paging_heap);
    page_operate_addr = hal_mmu_add_early_paging_addr(
                            (void*)(p_lv1_info->physical_addr), MMU_PAGE_RW);

    if(core_rtl_array_set(&mmu_globl_pg_table, 0, p_lv1_info) == NULL) {
        PANIC(ENOMEM,
              "Not enough memory for mmu paging managment.");
    }

    //Clear new lv1 table
    for(u32 i = 0;
        i < 4;
        i++) {
        switch_editing_page((void*)(p_lv1_info->physical_addr
                                    + SANDNIX_KERNEL_PAGE_SIZE * i));
        core_rtl_memset((void*)page_operate_addr, 0, SANDNIX_KERNEL_PAGE_SIZE);
    }

    //Set lv1 descriptor
    u32 used_lv2_table = used_lv2_desc / 256;

    if(used_lv2_desc % 256 != 0) {
        used_lv2_table++;
    }

    if(used_lv2_table % 4 > 0) {
        used_lv2_table += 4 - used_lv2_table % 4;
    }

    for(u32 i = KERNEL_MEM_BASE / 256 / 4096;
        i < KERNEL_MEM_BASE / 256 / 4096 + used_lv2_table;
        i++) {
        /*
         * lv1_pg_desc_t is 4 bytes, lv1 page table is 16KB. We can only map 4KB
         * once. So each lv2_tbl_info_t has 4 lv2 tables and use 4 lv1 descriptor.
         */
        //Map page
        switch_editing_page((void*)((p_lv1_info->physical_addr +
                                     i * sizeof(lv1_pg_desc_t)) & 0xFFFFF000));
        plv1_pg_desc_t p_lv1_desc = (plv1_pg_desc_t)page_operate_addr + i % 1024;
        LV1_DESC_TYPE_SET(p_lv1_desc, MMU_PG_LV2ENT);
        p_lv1_desc->lv2_entry.none_secure = 1;
        LV1_LV2ENT_SET_ADDR(p_lv1_desc,
                            (address_t)&init_lv2_pg_tbl[
                                (i - KERNEL_MEM_BASE / (256 * 4096)) * 256] + load_offset);
    }

    //Set lv2 descriptor
    used_lv2_table = used_lv2_table / 4;
    lv2_create0(used_lv2_table);
    return;
}

void switch_editing_page(void* phy_addr)
{
    plv2_pg_desc_t p_lv2_desc = &init_lv2_pg_tbl[((address_t)page_operate_addr
                                - KERNEL_MEM_BASE) / 4096];
    LV2_4KB_SET_ADDR(p_lv2_desc, (address_t)phy_addr);
    invalidate_TLB(page_operate_addr);
    return;
}

void lv2_create0(u32 used_lv2_table)
{
    for(u32 i = 0; i < used_lv2_table; i++) {
        plv2_tbl_info_t p_lv2_info = core_mm_heap_alloc(sizeof(lv2_tbl_info_t),
                                     mmu_paging_heap);

        if(p_lv2_info == NULL) {
            PANIC(ENOMEM,
                  "Not enough memory for mmu paging managment.");
        }

        p_lv2_info->freeable = false;
        p_lv2_info->ref = 0;
        p_lv2_info->physical_addr = (address_t)&init_lv2_pg_tbl[i * 256 * 4]
                                    + load_offset;

        for(u32 j = 0; j < 256 * 4; j++) {
            plv2_pg_desc_t p_lv2_desc = &init_lv2_pg_tbl[i * 256 * 4 + j];

            if(i * 4 * 256 + j < used_lv2_desc
               && (p_lv2_desc->value & 0x03) != 0) {
                //The lv2 descriptor has been used
                if((i * 4 * 256 + j) * 4096 + KERNEL_MEM_BASE
                   >= (address_t)(kernel_header.code_start)
                   && (i * 4 * 256 + j) * 4096 + KERNEL_MEM_BASE
                   < (address_t)(kernel_header.code_start) + kernel_header.code_size) {
                    //Code
                    LV2_SET_AP(p_lv2_desc, PG_AP_RO_NA);
                    p_lv2_desc->pg_4KB.xn = 0;

                } else {
                    //Data
                    LV2_SET_AP(p_lv2_desc, PG_AP_RW_NA);
                    p_lv2_desc->pg_4KB.xn = 1;
                }

                hal_early_print_printf("Virtual address %p -> physical address %p.\n",
                                       (i * 4 * 256 + j) * 4096 + KERNEL_MEM_BASE,
                                       LV2_4KB_GET_ADDR(p_lv2_desc));

            } else {
                //The lv2 descriptor is not been used.
                p_lv2_desc->value = 0;
            }
        }

        if(core_rtl_map_set(&mmu_krnl_lv2_tbl_map,
                            (void*)(i * 4096 * 256 * 4 + KERNEL_MEM_BASE),
                            p_lv2_info) == NULL) {
            PANIC(ENOMEM,
                  "Not enough memory for mmu paging managment.");
        }
    }

    return;
}

void remove_page(plv1_tbl_info_t p_lv1_info, void* virt_addr)
{
    plv2_tbl_info_t p_lv2_info;

    if((address_t)virt_addr >= KERNEL_MEM_BASE) {
        //Kernel page
        p_lv2_info = (plv2_tbl_info_t)core_rtl_map_get(
                         &mmu_krnl_lv2_tbl_map,
                         (void*)((address_t)virt_addr & 0xFFC00000));

    } else {
        //User page
        p_lv2_info = (plv2_tbl_info_t)core_rtl_map_get(
                         &(p_lv1_info->lv2_info_map),
                         (void*)((address_t)virt_addr & 0xFFC00000));
    }

    if(p_lv2_info == NULL) {
        return;
    }

    //Get lv2 descriptor
    switch_editing_page((void*)(p_lv2_info->physical_addr));
    plv2_pg_desc_t p_lv2_desc = (plv2_pg_desc_t)page_operate_addr
                                + (address_t)virt_addr % (4096 * 256 * 4) / 4096;

    if(LV2_DESC_TYPE(p_lv2_desc) == MMU_PG_FAULT) {
        return;
    }

    p_lv2_desc->value = 0;

    if(p_lv2_info->freeable) {
        (p_lv2_info->ref)--;

        if(p_lv2_info->ref == 0) {
            //Remove lv2 table
            if((address_t)virt_addr >= KERNEL_MEM_BASE) {
                core_rtl_map_set(&mmu_krnl_lv2_tbl_map,
                                 (void*)((address_t)virt_addr & 0xFFC00000),
                                 NULL);

            } else {
                core_rtl_map_set(&(p_lv1_info->lv2_info_map),
                                 (void*)((address_t)virt_addr & 0xFFC00000),
                                 NULL);

            }

            //Free lv2 table
            hal_mmu_phymem_free((void*)(p_lv2_info->physical_addr));
            core_mm_heap_free(p_lv2_info, mmu_paging_heap);

            //Unmap lv2 table in lv1 table
            switch_editing_page((void*)((address_t)(p_lv1_info->physical_addr)
                                        + (address_t)virt_addr / (4096 * 256)
                                        * sizeof(lv1_pg_desc_t)));

            for(u32 i = 0; i < 4; i++) {
                plv1_pg_desc_t p_lv1_desc = (plv1_pg_desc_t)(page_operate_addr)
                                            + (address_t)virt_addr % (4096 * 256) / 4 * 4 + i;
                p_lv1_desc->value = 0;
            }

            if((address_t)virt_addr >= KERNEL_MEM_BASE) {
                krnl_pdt_sync(p_lv1_info);
            }
        }
    }

    return;
}

kstatus_t set_page(plv1_tbl_info_t p_lv1_info, void* virt_addr, u32 attr,
                   void* phy_addr)
{
    plv2_tbl_info_t p_lv2_info;

    if((address_t)virt_addr >= KERNEL_MEM_BASE) {
        //Kernel page
        p_lv2_info = (plv2_tbl_info_t)core_rtl_map_get(
                         &mmu_krnl_lv2_tbl_map,
                         (void*)((address_t)virt_addr & 0xFFC00000));

    } else {
        //User page
        p_lv2_info = (plv2_tbl_info_t)core_rtl_map_get(
                         &(p_lv1_info->lv2_info_map),
                         (void*)((address_t)virt_addr & 0xFFC00000));
    }

    if(p_lv2_info == NULL) {
        //Create new lv2 table
        p_lv2_info = (plv2_tbl_info_t)core_mm_heap_alloc(sizeof(lv2_tbl_info_t),
                     mmu_paging_heap);

        if(p_lv2_info == NULL) {
            return ENOMEM;
        }

        p_lv2_info->freeable = true;
        p_lv2_info->ref = 0;

        if(hal_mmu_phymem_alloc((void**)(&(p_lv2_info->physical_addr)), 1,
                                false, 1) != ESUCCESS) {
            core_mm_heap_free(p_lv2_info, mmu_paging_heap);
            return ENOMEM;
        }

        switch_editing_page((void*)(p_lv2_info->physical_addr));
        core_rtl_memset((void*)page_operate_addr, 0, 4096);

        //Add new lv2 table
        if((address_t)virt_addr >= KERNEL_MEM_BASE) {
            //Kernel page
            if(core_rtl_map_set(&mmu_krnl_lv2_tbl_map,
                                (void*)((address_t)virt_addr & 0xFFC00000),
                                p_lv2_info) == NULL) {
                hal_mmu_phymem_free((void*)(p_lv2_info->physical_addr));
                core_mm_heap_free(p_lv2_info, mmu_paging_heap);
                return ENOMEM;
            }

        } else {
            //User page
            if(core_rtl_map_set(&(p_lv1_info->lv2_info_map),
                                (void*)((address_t)virt_addr & 0xFFC00000),
                                p_lv2_info) == NULL) {
                hal_mmu_phymem_free((void*)(p_lv2_info->physical_addr));
                core_mm_heap_free(p_lv2_info, mmu_paging_heap);
                return ENOMEM;
            }
        }

        //Add mapping
        switch_editing_page((void*)((address_t)(p_lv1_info->physical_addr)
                                    + (address_t)virt_addr / (4096 * 256)
                                    * sizeof(lv1_pg_desc_t)));

        for(u32 i = 0; i < 4; i++) {
            plv1_pg_desc_t p_lv1_desc = (plv1_pg_desc_t)(page_operate_addr)
                                        + (address_t)virt_addr / (4096 * 256) % 1024 + i;
            p_lv1_desc->value = 0;
            LV1_DESC_TYPE_SET(p_lv1_desc, MMU_PG_LV2ENT);
            p_lv1_desc->lv2_entry.none_secure = 1;
            LV1_LV2ENT_SET_ADDR(p_lv1_desc,
                                p_lv2_info->physical_addr + i * 256 * sizeof(lv2_pg_desc_t));
        }

        if((address_t)virt_addr >= KERNEL_MEM_BASE) {
            krnl_pdt_sync(p_lv1_info);
        }

    }

    //Set lv2 descriptor
    switch_editing_page((void*)(p_lv2_info->physical_addr));
    plv2_pg_desc_t p_lv2_desc = (plv2_pg_desc_t)page_operate_addr
                                + (address_t)virt_addr % (4096 * 256 * 4) / 4096;

    if(LV2_DESC_TYPE(p_lv2_desc) == MMU_PG_FAULT) {
        (p_lv2_info->ref)++;
    }

    LV2_DESC_TYPE_SET(p_lv2_desc, MMU_PG_LV2ENT);

    if((address_t)virt_addr >= KERNEL_MEM_BASE) {
        if(attr & MMU_PAGE_WRITABLE) {
            LV2_SET_AP(p_lv2_desc, PG_AP_RW_NA);

        } else {
            LV2_SET_AP(p_lv2_desc, PG_AP_RO_NA);
        }

    } else {
        if(attr & MMU_PAGE_WRITABLE) {
            LV2_SET_AP(p_lv2_desc, PG_AP_RW_RW);

        } else {
            LV2_SET_AP(p_lv2_desc, PG_AP_RO_RO);
        }

    }

    if(attr & MMU_PAGE_EXECUTABLE) {
        p_lv2_desc->pg_4KB.xn = 0;

    } else {
        p_lv2_desc->pg_4KB.xn = 1;
    }

    LV2_4KB_SET_ADDR(p_lv2_desc, (address_t)phy_addr);

    return ESUCCESS;
}

void krnl_pdt_sync(plv1_tbl_info_t p_lv1_info)
{
#define BEGIN_INDEX	(KERNEL_MEM_BASE / 4096 / 256)
    static plv1_pg_desc_t sync_buf[4096 - BEGIN_INDEX];

    //Copy to buffer
    for(u32 i = BEGIN_INDEX / 1024; i < 4; i++) {
        switch_editing_page((void*)(p_lv1_info->physical_addr + i * 4096));

        if(BEGIN_INDEX > i * 1024) {
            core_rtl_memcpy(sync_buf,
                            (plv1_pg_desc_t)page_operate_addr + BEGIN_INDEX - i * 1024,
                            (i + 1) * 1024 - BEGIN_INDEX);

        } else {
            core_rtl_memcpy(&sync_buf[i * 1024 - BEGIN_INDEX],
                            page_operate_addr,
                            4096);
        }
    }

    u32 index = 0;

    while(core_rtl_array_get_used_index(&mmu_globl_pg_table,
                                        index,
                                        &index)) {
        plv1_tbl_info_t p_lv1_info = (plv1_tbl_info_t)core_rtl_array_get(
                                         &mmu_globl_pg_table,
                                         index);

        for(u32 i = BEGIN_INDEX / 1024; i < 4; i++) {
            switch_editing_page((void*)(p_lv1_info->physical_addr + i * 4096));

            if(BEGIN_INDEX > i * 1024) {
                core_rtl_memcpy((plv1_pg_desc_t)page_operate_addr + BEGIN_INDEX - i * 1024,
                                sync_buf,
                                (i + 1) * 1024 - BEGIN_INDEX);

            } else {
                core_rtl_memcpy(page_operate_addr,
                                &sync_buf[i * 1024 - BEGIN_INDEX],
                                4096);
            }
        }

        index++;
    }

    return;
#undef BEGIN_INDEX
}

void lv2_info_destroy(plv2_tbl_info_t p_lv2_info, void* p_null)
{
    if(p_lv2_info->freeable) {
        hal_mmu_phymem_free((void*)(p_lv2_info->physical_addr));
    }

    core_mm_heap_free(p_lv2_info, mmu_paging_heap);
    UNREFERRED_PARAMETER(p_null);
    return;
}
