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

#include "../../../io/io.h"
#include "apic.h"
#include "interrupt.h"
#include "../../../../early_print/early_print.h"
#include "../../../../exception/exception.h"
#include "../../../../mmu/mmu.h"

#define	IA32_APIC_BASE	0x1B
#define	APIC_BASE_ADDR	0xFEE00000

static	u32					lvt_entry_num;

static	inline	bool		is_apic_supported();
static	inline	address_t	get_apic_phy_base();
static	inline	void		local_apic_init();
static	inline	void		io_apic_init();
static	inline	void		disable_apic();
static	inline	void		enable_apic();
static	inline	void		disbale_8259A();
static	inline	void		delay();

void apic_init()
{
    hal_early_print_printf("Initializing local APIC...\n");

    //Check if APIC is supported.
    if(!is_apic_supported()) {
        hal_exception_panic(ENOTSUP,
                            "APIC is not supported. Sandnix requires APIC to handle IRQ.");
    }

    //Map apic memory
    address_t apic_phy_base = get_apic_phy_base();
    hal_early_print_printf("APIC pysical base address = %p\n", apic_phy_base);
    hal_early_print_printf("Mapping pysical base address %p --> %p\n",
                           apic_phy_base,
                           APIC_BASE_ADDR);
    void* phy_addr;
    u32 attr;
    hal_mmu_pg_tbl_get(0, (void*)APIC_BASE_ADDR, &phy_addr, &attr);

    if(attr != MMU_PAGE_UNAVAIL) {
        hal_exception_panic(ENOMEM,
                            "Failed to map APIC memory.\n");
    }

    kstatus_t status = hal_mmu_pg_tbl_set(0, (void*)APIC_BASE_ADDR, MMU_PAGE_RW_NC,
                                          (void*)apic_phy_base);

    if(status != ESUCCESS) {
        hal_exception_panic(status, "Failed to map APIC memory.\n");
    }

    //Disable 8258A
    disbale_8259A();
    disable_apic();
    //Initialize local APIC
    local_apic_init();

    //Initialize io APIC
    io_apic_init();
    //enable_apic();

    return;
}

u32 hal_io_apic_read32(address_t off)
{
    return *((volatile u32*)(APIC_BASE_ADDR + off));
}

void hal_io_apic_write32(address_t off, u32 data)
{
    *((volatile u32*)(APIC_BASE_ADDR + off)) = data;
    return;
}

bool is_apic_supported()
{
    bool ret;

    __asm__ __volatile__(
        "movl	$1, %%eax\n"
        "cpuid\n"
        "bt		$9, %%edx\n"
        "setc	%%al\n"
        "movb	%%al, %0\n"
        :"=r"(ret)
        ::"ax", "cx");

    return ret;
}

address_t get_apic_phy_base()
{
    address_t ret;

    __asm__ __volatile__(
        "rdmsr\n"
        "andl	$0xfffff000, %%eax\n"
        "andl	$0x0f, %%edx\n"
        "shll	$32, %%edx\n"
        "orl	%%edx, %%eax\n"
        :"=ax"(ret)
        :"cx"((u32)IA32_APIC_BASE)
        :"dx");

    return ret;
}


void local_apic_init()
{
    //Get local apic information
    //Number of LVT Entery
    u32 ver_reg = hal_io_apic_read32(LOCAL_APIC_VERSION_REG);
    lvt_entry_num = ((ver_reg & 0xFF0000) >> 16) + 1;
    hal_early_print_printf("Number of LVT entery : %d.\n", lvt_entry_num);

    //Allow all interrupt
    hal_io_apic_write32(LOCAL_APIC_TPR_REG,
                        hal_io_apic_read32(LOCAL_APIC_TPR_REG) & 0xFFFFFF0F);

    //Disable LVT0
    hal_io_apic_write32(LOCAL_APIC_LVT_LINT0_REG,
                        hal_io_apic_read32(LOCAL_APIC_LVT_LINT0_REG) | 0x10000);
}

void io_apic_init()
{
}

void disable_apic()
{
    hal_io_apic_write32(LOCAL_APIC_SVR_REG,
                        hal_io_apic_read32(LOCAL_APIC_SVR_REG) & (~0x100));
    return;
}

void enable_apic()
{
    hal_io_apic_write32(LOCAL_APIC_SVR_REG,
                        hal_io_apic_read32(LOCAL_APIC_SVR_REG) | 0x100);
    return;
}

void disbale_8259A()
{
    hal_early_print_printf("Disabling 8259A...\n");
    //Master ICW1
    hal_io_out_8(0x20, 0x11);
    delay();

    //Slave	ICW1
    hal_io_out_8(0xA0, 0x11);
    delay();

    //Master ICW2
    hal_io_out_8(0x21, 0x20);
    delay();

    //Slave ICW2
    hal_io_out_8(0xA1, 0x28);
    delay();

    //Master ICW3
    hal_io_out_8(0x21, 0x04);
    delay();

    //Slave ICW3
    hal_io_out_8(0xA1, 0x02);
    delay();

    //Master ICW4
    hal_io_out_8(0x21, 0x01);
    delay();

    //Slave ICW4
    hal_io_out_8(0xA1, 0x01);
    delay();

    //Master OCW1
    hal_io_out_8(0x21, 0xFF);
    delay();

    //Slave OCW1
    hal_io_out_8(0xA1, 0xFF);
    delay();
    return;
}

void delay()
{
    __asm__ __volatile__(
        "nop\n"
        "nop\n"
        "nop\n"
        "nop\n"
        :::);
    return;
}
