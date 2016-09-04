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
#include "../../../../rtl/rtl.h"

#define	IA32_APIC_BASE	0x1B

static	u32					lvt_entry_num;
static	address_t			local_apic_base;

//IO APIC registers
static	address_t			ioapic_base;
#define	ioapic_index_reg	((volatile u8*)(ioapic_base))
#define	ioapic_data_reg		((volatile u32*)(ioapic_base + 0x10))
#define	ioapic_EOI_reg		((volatile u32*)(ioapic_base + 0x40))

//HPET registers
static	address_t			hpet_base_address;
#define	hpet_id_reg			((volatile u64*)(hpet_base_address + 0x00))
#define	hpet_configure_reg	((volatile u64*)(hpet_base_address + 0x10))
#define	hpet_status_reg		((volatile u64*)(hpet_base_address + 0x20))
#define	hpet_counter_reg	((volatile u64*)(hpet_base_address + 0xF0))

//Timer
#define	hpet_timer_cfg_reg(n)	((volatile u64*)(hpet_base_address + 0x100 \
                                 + 0x20 * (n)))
#define	hpet_timer_comp_reg(n)	((volatile u64*)(hpet_base_address + 0x108 \
                                 + 0x20 * (n)))

#define	HPET_COUNT_FS			((*hpet_id_reg) >> 32)
static	u32					timer_period;

//RCBA registers
static	address_t			rcba_address;
static	volatile u16*		oic_addr;
static	volatile u32*		rcba_hpet_cfg_reg;;

static	inline	bool		is_apic_supported();
static	inline	address_t	get_apic_phy_base();
static	inline	void		local_apic_init();
static	inline	void		io_apic_init();
static	inline	void		clock_init();
static	inline	void		disable_apic();
static	inline	void		enable_apic();
static	inline	void		disbale_8259A();
static	inline	void		delay();

static	inline	u32			ioapic_id_read();
static	inline	void		ioapic_id_write(u32 data);
static	inline	u32			ioapic_version_read();
static	inline	u64			ioapic_redirct_tbl_read(u8 index);
static	inline	void		ioapic_redirct_tbl_write(u8 index, u64 data);

static	inline	void		tick_init();
static	inline	u32			lapic_speed();
static	void				timer_init_int(u32 int_num, pcontext_t p_context,
        u32 err_code);

void apic_init()
{
    hal_early_print_printf("Initializing local APIC...\n");

    //Check if APIC is supported.
    if(!is_apic_supported()) {
        PANIC(ENOTSUP,
              "APIC is not supported. Sandnix requires APIC to handle IRQ.");
    }

    //Map apic memory
    address_t apic_phy_base = get_apic_phy_base();
    hal_early_print_printf("APIC pysical base address = %p\n", apic_phy_base);
    hal_early_print_printf("Mapping pysical address %p --> %p\n",
                           apic_phy_base,
                           local_apic_base);

    local_apic_base = (address_t)hal_mmu_add_early_paging_addr((void*)(apic_phy_base),
                      MMU_PAGE_RW_NC);

    //Disable 8258A
    disbale_8259A();
    disable_apic();
    //Initialize local APIC
    local_apic_init();

    //Initialize io APIC
    io_apic_init();

    //Initialize clock
    clock_init();

    enable_apic();

    //Initialize tick
    tick_init();

    return;
}

void hal_io_irq_enable_all()
{
    ioapic_redirect_entry_t redict_entry_value;
    u32 redirct_tbl_num = (ioapic_version_read() & 0xFF0000) >> 16;

    for(u32 i = 0; i < redirct_tbl_num; i++) {
        redict_entry_value.value = ioapic_redirct_tbl_read(i);
        redict_entry_value.data.mask = 0;
        ioapic_redirct_tbl_write(i, redict_entry_value.value);
    }

    return;
}

void hal_io_irq_disable_all()
{
    ioapic_redirect_entry_t redict_entry_value;
    u32 redirct_tbl_num = (ioapic_version_read() & 0xFF0000) >> 16;

    for(u32 i = 0; i < redirct_tbl_num; i++) {
        redict_entry_value.value = ioapic_redirct_tbl_read(i);
        redict_entry_value.data.mask = 1;
        ioapic_redirct_tbl_write(i, redict_entry_value.value);
    }

    return;
}

void hal_io_irq_enable(u32 num)
{
    ioapic_redirect_entry_t redict_entry_value;

    if(num >= IRQ_BASE && num <= IRQ_MAX) {
        redict_entry_value.value = ioapic_redirct_tbl_read(num - IRQ_BASE);
        redict_entry_value.data.mask = 0;
        ioapic_redirct_tbl_write(num - IRQ_BASE, redict_entry_value.value);
    }

    return;
}

void hal_io_irq_disable(u32 num)
{
    ioapic_redirect_entry_t redict_entry_value;

    if(num >= IRQ_BASE && num <= IRQ_MAX) {
        redict_entry_value.value = ioapic_redirct_tbl_read(num - IRQ_BASE);
        redict_entry_value.data.mask = 1;
        ioapic_redirct_tbl_write(num - IRQ_BASE, redict_entry_value.value);
    }

    return;
}

void hal_io_get_irq_range(u32* p_begin, u32* p_num)
{
    *p_begin = IRQ_BASE;
    *p_num = IRQ_MAX - IRQ_BASE + 1;
    return;
}

u32 hal_io_apic_read32(address_t off)
{
    return *((volatile u32*)(local_apic_base + off));
}

void hal_io_apic_write32(address_t off, u32 data)
{
    *((volatile u32*)(local_apic_base + off)) = data;
    return;
}

void hal_io_irq_send_eoi()
{
    hal_io_apic_write32(LOCAL_APIC_EOI_REG, 0);
    return;
}

void hal_io_set_clock_period(u32 microsecond)
{
    timer_period = microsecond;

    u64 val = hal_rtl_math_div64((u64)microsecond * 1000
                                 * 1000 * 1000,
                                 HPET_COUNT_FS);
    *hpet_configure_reg &= ~(u64)0x01;
    *hpet_timer_comp_reg(0) = val;
    *hpet_configure_reg |= 0x01;

    return;
}

u32 hal_io_get_clock_period()
{
    return timer_period;
}

u32 hal_io_get_max_clock_period()
{
    u64 ret = 0x10000000 * HPET_COUNT_FS - 1;

    if(ret < 0x100000000) {
        return ret;

    } else {
        return 0xFFFFFFFF;
    }
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
    //Get OIC address
    hal_io_out_32(PCI_CONFIG_ADDRESS,
                  ((u32)0x80000000 | (u32)(0 << 16) | (u32)(31 << 11)
                   | (u32)(0 << 8) | (u32)(0xF0 & 0xfc)));
    rcba_address = hal_io_in_32(PCI_CONFIG_DATA);


    if(rcba_address == 0xFFFFFFFF) {
        //Bus 0, device 31, offset 0xF0 does not exists.
        PANIC(ENOTSUP,
              "PCI bus 0, device 31, function 0, offset 0xF0 does not exists. "
              "Sandnix requires ICH9 support.\n");

    }

    rcba_address = rcba_address & 0xFFFFC000;
    address_t oic_phy_addr = (rcba_address) + 0x31FE ;
    hal_early_print_printf("OIC physical address = %p.\n", oic_phy_addr);
    oic_addr = (volatile u16*)hal_mmu_add_early_paging_addr((void*)oic_phy_addr,
               MMU_PAGE_RW_NC);
    hal_early_print_printf("Mapping pysical address %p --> %p\n",
                           oic_phy_addr,
                           oic_addr);
    hal_early_print_printf("OIC value = %#.4X.\n", *oic_addr);
    address_t ioapic_phy_base = (((*oic_addr) & 0xFF) << 12) | 0xFEC00000;

    //Enable IOAPIC
    *oic_addr = *oic_addr | 0x100;

    //Get IOAPIC base address
    ioapic_base = (address_t)hal_mmu_add_early_paging_addr(
                      (void*)ioapic_phy_base,
                      MMU_PAGE_RW_NC);
    hal_early_print_printf("Mapping pysical address %p --> %p\n",
                           ioapic_phy_base,
                           ioapic_base);

    //Initialize IOAPIC
    ioapic_id_write((ioapic_id_read() & ~(u32)0xF000000));
    hal_early_print_printf("IOAPIC version : %#.2X.\n",
                           ioapic_version_read() & 0xFF);

    //Get number of redirection table
    u32 redirct_tbl_num = (ioapic_version_read() & 0xFF0000) >> 16;
    redirct_tbl_num++;
    hal_early_print_printf("Number of IOAPIC redirection table : %u.\n",
                           redirct_tbl_num);

    //Initialize redirection table
    ioapic_redirect_entry_t redict_entry_value;
    core_rtl_memset(&redict_entry_value, 0, sizeof(redict_entry_value));

    for(u32 i = 0; i < redirct_tbl_num; i++) {
        redict_entry_value.data.vector = IRQ_BASE + i;
        ioapic_redirct_tbl_write(i, redict_entry_value.value);
        hal_early_print_printf("IRQ%u --> INT %#.2X.\n", i, IRQ(i));
    }

    return;
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

u32 ioapic_id_read()
{
    *ioapic_index_reg = 0x00;
    return *ioapic_data_reg;
}

void ioapic_id_write(u32 data)
{
    *ioapic_index_reg = 0x00;
    *ioapic_data_reg = data;
    return;
}

u32 ioapic_version_read()
{
    *ioapic_index_reg = 0x01;
    return *ioapic_data_reg;
}

u64 ioapic_redirct_tbl_read(u8 index)
{
    index = index * 2;
    u64 ret = 0;
    *ioapic_index_reg = 0x10 + index;
    ret = *ioapic_data_reg;
    *ioapic_index_reg = 0x10 + index + 1;
    ret = (ret << 32) + *ioapic_data_reg;

    return ret;
}

void ioapic_redirct_tbl_write(u8 index, u64 data)
{
    index = index * 2;
    *ioapic_index_reg = 0x10 + index;
    *ioapic_data_reg = (u32)(data & 0xFFFFFFFF);
    *ioapic_index_reg = 0x10 + index + 1;
    *ioapic_data_reg = (u32)(data >> 32);

    return;
}

void clock_init()
{
    //Get address
    hal_early_print_printf("Initializing HPET...\n");
    rcba_hpet_cfg_reg = (volatile u32*)hal_mmu_add_early_paging_addr(
                            (void*)(rcba_address + 0x3404), MMU_PAGE_RW_NC);
    hal_early_print_printf("Mapping physical address : %p --> %p.\n",
                           rcba_address + 0x3404,
                           rcba_hpet_cfg_reg);
    *rcba_hpet_cfg_reg = 0x80;

    hpet_base_address = (address_t)hal_mmu_add_early_paging_addr(
                            (void*)0xFED00000, MMU_PAGE_RW_NC);

    //Initialize HPET
    hal_early_print_printf("HPET ID Register = %#.16llx.\n", *hpet_id_reg);

    //Disable HPET
    *hpet_configure_reg = (*hpet_configure_reg | 0x02) & (~(u64)0x01);
    *hpet_counter_reg = 0;

    //Disable all timer
    for(u32 i = 0; i <= 7; i++) {
        *hpet_timer_cfg_reg(i) &= ~(u64)0x04;
    }

    //Initialize timer#0
    hal_early_print_printf("Initializing timer#0...\n");
    *hpet_counter_reg = 0;
    hal_io_set_clock_period(10000);
    *hpet_timer_cfg_reg(0) = 0x4C;

    //Enable HPET
    *hpet_configure_reg |= 0x01;

    return;
}

void tick_init()
{
    //Inicialize local apic timer
    hal_early_print_printf("Initialize local apic timer...\n");
    hal_io_apic_write32(LOCAL_APIC_TIMER_INITIAL_COUNT_REG, 0);
    hal_io_apic_write32(LOCAL_APIC_TIMER_CURRENT_COUNT_REG, 0);
    u32 div_cfg = hal_io_apic_read32(LOCAL_APIC_TIMER_DIVIDE_CONF_REG);
    div_cfg |= 0x0B;
    hal_io_apic_write32(LOCAL_APIC_TIMER_DIVIDE_CONF_REG, div_cfg);

    u32 timer_reg_val = hal_io_apic_read32(LOCAL_APIC_LVT_TIMER_REG);
    timer_reg_val &= ~0x710FF;
    timer_reg_val |= 0x20000 | (INT_TICK & 0xFF);
    hal_io_apic_write32(LOCAL_APIC_LVT_TIMER_REG, timer_reg_val);

    //Compute how many counts a tick.
    u32 count_per_tick = lapic_speed();
    hal_early_print_printf("%u counts per tick...\n");

    //Enable timer
    hal_io_apic_write32(LOCAL_APIC_TIMER_INITIAL_COUNT_REG, count_per_tick);

    return;
}

static	volatile	u32 	timer_interrupted_count;
u32 lapic_speed()
{
    //Set temporart interrupt call back
    hal_io_int_callback_set(IRQ_CLOCK, timer_init_int);
    hal_io_int_disable();
    timer_interrupted_count = 0;

    //Set clock period
    hal_io_set_clock_period(TICK_PERIOD);

    //Stop counting
    hal_io_apic_write32(LOCAL_APIC_TIMER_INITIAL_COUNT_REG, 0);
    hal_io_apic_write32(LOCAL_APIC_TIMER_CURRENT_COUNT_REG, 0);
    hal_io_int_enable();

    //Wait for first interrupt
    u32 old_count = timer_interrupted_count;

    while(old_count == timer_interrupted_count) {
        MEM_BLOCK;
    }

    //Start counting
    hal_io_apic_write32(LOCAL_APIC_TIMER_INITIAL_COUNT_REG, 0xFFFFFFFF);

    //Wait for second interrupt
    old_count = timer_interrupted_count;

    while(old_count == timer_interrupted_count) {
        MEM_BLOCK;
    }

    u32 ret = 0xFFFFFFFF - hal_io_apic_read32(LOCAL_APIC_TIMER_CURRENT_COUNT_REG);
    hal_io_apic_write32(LOCAL_APIC_TIMER_INITIAL_COUNT_REG, 0);
    hal_io_apic_write32(LOCAL_APIC_TIMER_CURRENT_COUNT_REG, 0);
    hal_io_int_disable();
    hal_io_int_callback_set(IRQ_CLOCK, NULL);

    return ret;
}

void timer_init_int(u32 int_num, pcontext_t p_context,
                    u32 err_code)
{
    timer_interrupted_count++;
    UNREFERRED_PARAMETER(int_num);
    UNREFERRED_PARAMETER(p_context);
    UNREFERRED_PARAMETER(err_code);
    return;
}
