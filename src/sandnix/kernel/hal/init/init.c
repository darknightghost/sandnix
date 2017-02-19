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

#include "../../core/main/main.h"
#include "../../core/rtl/rtl.h"
#include "../../core/mm/mm.h"
#include "../../core/pm/pm.h"

#include "../early_print/early_print.h"
#include "../mmu/mmu.h"
#include "../exception/exception.h"
#include "../kparam/kparam.h"
#include "../io/io.h"
#include "../cpu/cpu.h"
#include "../sys_gate/sys_gate.h"

#include "init.h"

#if defined(X86)
    #include "./arch/x86/header.h"
#elif defined(ARM)
    #include "./arch/arm/header.h"
#endif

u8	__attribute__((aligned(4096)))	init_stack[DEFAULT_STACK_SIZE];

#define	MODULE_NAME		hal_init

void kinit(void* p_bootloader_info)
{
    PRIVATE(arch_init)();
    hal_early_print_init();
    hal_early_print_printf("%s loading...\n", VER_STR);

    //Analyse bootloader parameters
    PRIVATE(analyse_bootloader_info)(p_bootloader_info);
    hal_kparam_init();

    //Initialize modules
    hal_io_init();
    hal_cpu_init();
    hal_mmu_init();
    hal_exception_init();
    hal_debug_init();
    //hal_sys_gate_init();

    //Enable interrupts
    hal_io_int_enable();
    hal_io_irq_enable_all();

    //Call kernel main
    hal_early_print_printf("\nCalling kernel main...\n\n");
    core_main_main();

    //Never return, in fact.
    return;
}
