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
#include "../../../../../init/init.h"

/*
 *   Though ARM supports different size of pages, we only use 4KB pages in order
 * to make our kernel compatibal with different architectures of CPUs.
 */

/*
 *   On this condition, lv1 table has 16KB / 4 bytes = 4096 entries,
 * lv2 table has 1KB / 4 bytes = 256 entries.
 * Total memory is 256 * 4096 *4096 bytes = 4GB
 */

#define	REQUIRED_INIT_PAGE_NUM	((KERNEL_MAX_SIZE) / 4096 + 1)
#define	MAX_INIT_PAGE_NUM		(REQUIRED_INIT_PAGE_NUM % 256 > 0 \
                                 ? (REQUIRED_INIT_PAGE_NUM / 256 + 1) * 256 \
                                 : REQUIRED_INIT_PAGE_NUM)

static lv1_pg_desc_t __attribute__((aligned(1024 * 16)))	init_lv1_pg_tbl[4096];
static lv2_pg_desc_t __attribute__((aligned(1024)))			init_lv2_pg_tbl[MAX_INIT_PAGE_NUM];
static u32			used_lv2_desc;
static address_t	load_offset;

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
static inline void	invalidate_TLB();
static inline void	invalidate_icache();
static inline void	invalidate_dcache();
static inline void	load_TTBR(address_t phyaddr);

void start_paging()
{
    if(initialized) {
        //TODO:panic
    }

    //Compute offset
    address_t offset;
    __asm__ __volatile__(
        "_ADDR1:\n"
        "mov	%0, pc\n"
        "sub	%0, %0, #8\n"
        "ldr	r0, =_ADDR1\n"
        "sub	%0, %0, r0\n"
        :"=r"(offset)
        ::"r0");

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
    invalidate_TLB();

    //Start paging
    invalidate_icache();
    enable_mmu();
    enable_dcache();
    enable_icache();

    load_offset = offset;
    return;
}

void* hal_mmu_add_early_paging_addr(void* phy_addr)
{
    u32 desc_type;

    if(initialized) {
        //TODO:panic
    }

    address_t pg_addr = (address_t)phy_addr;
    pg_addr -= pg_addr % 4096;
    address_t addr_off = (address_t)phy_addr - pg_addr;

    //Check if the address has been mapped
    u32 i;

    for(i = 0; i < used_lv2_desc; i++) {
        if(LV2_DESC_TYPE(&init_lv2_pg_tbl[i], desc_type) == MMU_PG_4KB) {
            if(LV2_4KB_GET_ADDR(&init_lv2_pg_tbl[i]) == pg_addr) {
                //The address has been mapped
                return (void*)(KERNEL_MEM_BASE + i * 4096 + addr_off);
            }
        }
    }

    //Get a free page
    if(used_lv2_desc >= MAX_INIT_PAGE_NUM) {
        //TODO:panic
    }

    used_lv2_desc++;

    //Fill the page descriptor
    LV2_DESC_TYPE_SET(&init_lv2_pg_tbl[i], MMU_PG_4KB);
    init_lv2_pg_tbl[i].pg_4KB.xn = 0;
    init_lv2_pg_tbl[i].pg_4KB.reserv1 = 0;
    init_lv2_pg_tbl[i].pg_4KB.reserv2 = 0;
    init_lv2_pg_tbl[i].pg_4KB.reserv3 = 0;
    LV2_SET_AP(&init_lv2_pg_tbl[i], PG_AP_RW_NA);
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

    invalidate_TLB();

    return (void*)(KERNEL_MEM_BASE + i * 4096 + addr_off);
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

    if(fill_num % 256 != 0) {
        fill_num = fill_num + (256 - fill_num % 256);
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

    used_lv2_desc = pg_num;
    return;
}

void init_SCTLR()
{
#define	MASK	0x62007405
#define	VAL		0x00006400
    /*
     * Theses bits of SCTLR is setted to:
     *		Bit 0(M)		: 0
     *		Bit 2(C)		: 0
     *		Bit 10(SW)		: 1
     *		Bit	12(I)		: 0
     *		Bit 13(V)		: 1
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

void invalidate_TLB()
{
    __asm__  __volatile__(
        "mov    r0, #0\n"
        "mcr    p15, 0, r0, c8, c7, 0\n"
        :::"r0");
    return;
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
