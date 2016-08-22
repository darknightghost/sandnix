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

#include "init.h"
#include "../early_print/early_print.h"
#include "../mmu/mmu.h"
#include "../../core/rtl/rtl.h"
#include "../../hal/exception/exception.h"
#include "../../core/mm/mm.h"
#include "../../hal/kparam/kparam.h"

#if defined(X86)
    #include "./arch/x86/header.h"
#elif defined(ARM)
    #include "./arch/arm/header.h"
#endif

u8	__attribute__((aligned(4096)))	init_stack[DEFAULT_STACK_SIZE];
void kinit(void* p_bootloader_info)
{
    hal_early_print_init();
    hal_early_print_printf("%s loading...\n", VER_STR);

    //Analyse bootloader parameters
    analyse_bootloader_info(p_bootloader_info);
    hal_kparam_init();

    //Initialize modules
    hal_mmu_init();

    void test();
    test();

    while(1);

    return;
}

void test()
{
    void* phy_addr;
    u32 attr;

    hal_early_print_printf("test\n");
    kstatus_t status = hal_mmu_pg_tbl_set(0, (void*)0xD0000000,
                                          MMU_PAGE_RW_EXEC, (void*)0x50000000);

    hal_mmu_pg_tbl_get(0, (void*)(0xD0000000), &phy_addr, &attr);

    if(attr != 0) {
        hal_early_print_printf("%p --> %p,%p\n",
                               (void*)(0xD0000000),
                               phy_addr,
                               attr);
    }

    UNREFERRED_PARAMETER(status);

    return;
}
