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


#include "apic.h"
#include "interrupt.h"
#include "../../../../early_print/early_print.h"
#include "../../../../exception/exception.h"

#define	IA32_APIC_BASE	0x1B

static	inline	bool		is_apic_supported();
static	inline	address_t	get_apic_phy_base();

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
