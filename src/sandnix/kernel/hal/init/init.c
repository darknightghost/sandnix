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
#include "../exception/exception.h"
#include "../kparam/kparam.h"
#include "../io/io.h"
#include "../../core/rtl/rtl.h"
#include "../../core/mm/mm.h"

#if defined(X86)
    #include "./arch/x86/header.h"
#elif defined(ARM)
    #include "./arch/arm/header.h"
#endif

u8	__attribute__((aligned(4096)))	init_stack[DEFAULT_STACK_SIZE];
void kinit(void* p_bootloader_info)
{
    arch_init();
    hal_early_print_init();
    hal_early_print_printf("%s loading...\n", VER_STR);

    //Analyse bootloader parameters
    analyse_bootloader_info(p_bootloader_info);
    hal_kparam_init();

    //Initialize modules
    hal_io_init();
    hal_cpu_init();
    hal_mmu_init();

    void test();
    test();

    hal_io_int_enable();
    //hal_io_irq_enable_all();

    while(1);

    return;
}

volatile u32 tm = 3000;
/*
void keyboard_int(u32 int_num, pcontext_t p_context, u32 err_code)
{
    tm = 3000;
    hal_io_in_8(I8408_DATA_PORT);
    hal_io_in_8(I8408_DATA_PORT);
    hal_io_in_8(I8408_DATA_PORT);
    UNREFERRED_PARAMETER(int_num);
    UNREFERRED_PARAMETER(p_context);
    UNREFERRED_PARAMETER(err_code);
}
*/

void ipi_hndlr(pcontext_t p_context, void* p_arg)
{
    tm--;

    if(tm == 0) {
        tm = 3000;
    }

    UNREFERRED_PARAMETER(p_context);
    UNREFERRED_PARAMETER(p_arg);
}

void clock_int(u32 int_num, pcontext_t p_context, u32 err_code)
{
    hal_cpu_send_IPI(0, 0, NULL);
    UNREFERRED_PARAMETER(int_num);
    UNREFERRED_PARAMETER(p_context);
    UNREFERRED_PARAMETER(err_code);
}

void tick_int(u32 int_num, pcontext_t p_context, u32 err_code)
{
    hal_early_print_printf("\r%.4u", tm);
    UNREFERRED_PARAMETER(int_num);
    UNREFERRED_PARAMETER(p_context);
    UNREFERRED_PARAMETER(err_code);
}

void test()
{
    hal_early_print_printf("Test\n");
    hal_io_set_clock_period(1000000);
    hal_io_int_callback_set(INT_CLOCK, clock_int);
    hal_io_int_callback_set(INT_TICK, tick_int);
    hal_cpu_regist_IPI_hndlr(0, ipi_hndlr);

    return;
}
