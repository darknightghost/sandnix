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

#include "gic.h"
#include "../../../../../../../mmu/mmu.h"
#include "../../../../../../../early_print/early_print.h"

#define	GIC_CONTROLLER_PHY_BASE		0x10480000
#define	GIC_DISTRIBUTOR_PHY_BASE	0x10490000

static		address_t	gic_controller_bases[4];
static		address_t	gic_distributor_base[4];

#define	TO_REG(addr)	(*((volatile u32*)(void*)((addr))))

//GIC controller registers
#define	ICCICR_CPU(n)			TO_REG(gic_controller_bases[(n)] + 0x00)
#define	ICCPMR_CPU(n)			TO_REG(gic_controller_bases[(n)] + 0x04)
#define	ICCBPR_CPU(n)			TO_REG(gic_controller_bases[(n)] + 0x08)
#define	ICCIAR_CPU(n)			TO_REG(gic_controller_bases[(n)] + 0x0C)
#define	ICCEOI_CPU(n)			TO_REG(gic_controller_bases[(n)] + 0x10)
#define	ICCRPR_CPU(n)			TO_REG(gic_controller_bases[(n)] + 0x14)
#define	ICCHPIR_CPU(n)			TO_REG(gic_controller_bases[(n)] + 0x18)
#define	ICCABPR_CPU(n)			TO_REG(gic_controller_bases[(n)] + 0x1C)
#define	INTEG_EN_C_CPU(n)		TO_REG(gic_controller_bases[(n)] + 0x40)
#define	INTERRUPT_OUT_CPU(n)	TO_REG(gic_controller_bases[(n)] + 0x44)
#define	ICCIIDR					TO_REG(gic_controller_bases[0] + 0xFC)

//GIC distributer registers
#define	ICDDCR					TO_REG(gic_distributor_base[0] + 0x00)
#define	ICDICTR					TO_REG(gic_distributor_base[0] + 0x04)
#define	ICDIIDR					TO_REG(gic_distributor_base[0] + 0x08)
#define	ICDISR_SPI(n)			TO_REG(gic_distributor_base[0] + 0x84 + 4 * (n))
#define	ICDISER_SPI(n)			TO_REG(gic_distributor_base[0] + 0x104 + 4 * (n))
#define	ICDICER_SPI(n)			TO_REG(gic_distributor_base[0] + 0x184 + 4 * (n))
#define	ICDISPR_SPI(n)			TO_REG(gic_distributor_base[0] + 0x204 + 4 * (n))
#define	ICDICPR_SPI(n)			TO_REG(gic_distributor_base[0] + 0x284 + 4 * (n))
#define	ICDIABR_SPI(n)			TO_REG(gic_distributor_base[0] + 0x304 + 4 * (n))
#define	ICDIPR_SPI(n)			TO_REG(gic_distributor_base[0] + 0x420 + 4 * (n))
#define	ICDIPTR_SPI(n)			TO_REG(gic_distributor_base[0] + 0x820 + 4 * (n))
#define	ICDICFR_SPI(n)			TO_REG(gic_distributor_base[0] + 0xC08 + 4 * (n))
#define	SPI_STATUS(n)			TO_REG(gic_distributor_base[0] + 0xD04 + 4 * (n))
#define	ICDSGIR					TO_REG(gic_distributor_base[0] + 0xF00)

#define	ICDISR_SGI_PPI(cpu)		TO_REG(gic_distributor_base[(cpu)] + 0x80)
#define	ICDISER_SGI_PPI(cpu)	TO_REG(gic_distributor_base[(cpu)] + 0x100)
#define	ICDICER_SGI_PPI(cpu)	TO_REG(gic_distributor_base[(cpu)] + 0x180)
#define	ICDISPR_SGI_PPI(cpu)	TO_REG(gic_distributor_base[(cpu)] + 0x200)
#define	ICDICPR_SGI_PPI(cpu)	TO_REG(gic_distributor_base[(cpu)] + 0x280)
#define	ICDABR_SGI_PPI(cpu)		TO_REG(gic_distributor_base[(cpu)] + 0x300)
#define	ICDIPR_SGI_PPI(cpu, n)	TO_REG(gic_distributor_base[(cpu)] + 0x400 \
                                       + 4 * (n))
#define	ICDIPTR_SGI(cpu, n)		TO_REG(gic_distributor_base[(cpu)] + 0x800 \
                                       + 4 * (n))

void gic_init()
{
    //Map registers
    for(u32 i = 0; i < 4; i++) {
        void* phy_base = (void*)(GIC_CONTROLLER_PHY_BASE + i * 0x4000);
        gic_controller_bases[i] = (address_t)hal_mmu_add_early_paging_addr(
                                      phy_base,
                                      MMU_PAGE_RW_NC);
        hal_early_print_printf("Mapping physical address %p->%p.\n",
                               phy_base, gic_controller_bases[i]);
    }

    for(u32 i = 0; i < 4; i++) {
        void* phy_base = (void*)(GIC_DISTRIBUTOR_PHY_BASE + i * 0x4000);
        gic_distributor_base[i] = (address_t)hal_mmu_add_early_paging_addr(
                                      phy_base,
                                      MMU_PAGE_RW_NC);
        hal_early_print_printf("Mapping physical address %p->%p.\n",
                               phy_base, gic_controller_bases[i]);
    }

    //Initialize GIC
    ICDDCR = 0x01;

    for(u32 i = 0; i < 4; i++) {
        ICDISR_SPI(i) = 0xFFFFFFFF;
        ICDISER_SPI(i) = 0xFFFFFFFF;
    }

    //Set IRQ targets
    for(u32 i = 0; i < 32; i++) {
        ICDIPTR_SPI(i) = 0x1010101;
    }

    for(u32 i = 0; i < 4; i++) {
        ICCPMR_CPU(i) = 0x00;
        ICCBPR_CPU(i) = 0x00;
        ICCRPR_CPU(i) = 0xFF;
        ICCABPR_CPU(i) = 0x00;
        INTEG_EN_C_CPU(i) = 0x00;
        INTERRUPT_OUT_CPU(i) = 0x00;
        ICDISR_SGI_PPI(i) = 0xFFFFFFFF;
        ICDISER_SGI_PPI(i) = 0xFFFFFFFF;

        //Enable GIC
        ICCICR_CPU(i) = 0x01;
    }

    return;
}

u32 gic_get_irq_num()
{
    return 0;
}

u32 hal_io_gic_get_cpu_id()
{
    return ICCIIDR;
}
