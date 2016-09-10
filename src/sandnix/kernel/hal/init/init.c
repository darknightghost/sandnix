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
    //hal_cpu_init();
    //hal_mmu_init();

    void test();
    test();

    hal_io_irq_enable_all();
    hal_io_int_enable();

    while(1) {
        hal_io_int_enable();
    }

    return;
}

static volatile u32 tm = 3000;
void ipi_hndlr(pcontext_t p_context, void* p_args)
{
    hal_early_print_printf("\r%u", tm);
    UNREFERRED_PARAMETER(p_context);
    UNREFERRED_PARAMETER(p_args);
    return;
}

void int_key(u32 int_num, pcontext_t p_context, u32 err_code)
{
    tm = 3000;

    hal_io_in_8(I8408_DATA_PORT);
    hal_io_in_8(I8408_DATA_PORT);
    hal_io_in_8(I8408_DATA_PORT);
    hal_io_irq_send_eoi();
    hal_cpu_context_load(p_context);
    UNREFERRED_PARAMETER(int_num);
    UNREFERRED_PARAMETER(p_context);
    UNREFERRED_PARAMETER(err_code);
    return;
}

void int_clk(u32 int_num, pcontext_t p_context, u32 err_code)
{
    if(tm == 0) {
        tm = 3000;

    } else {
        tm--;
    }

    hal_early_print_printf("\r%u", tm);

    hal_io_irq_send_eoi();
    hal_cpu_context_load(p_context);
    UNREFERRED_PARAMETER(int_num);
    UNREFERRED_PARAMETER(p_context);
    UNREFERRED_PARAMETER(err_code);
    return;
}

void test()
{
    //hal_cpu_regist_IPI_hndlr(0, ipi_hndlr);
    //hal_io_set_clock_period(1000);
    hal_io_int_callback_set(INT_CLOCK, int_clk);
    hal_io_int_callback_set(IRQ(1), int_key);
    /*
        while(1) {
            hal_cpu_send_IPI(0, 0, NULL);

            for(u32 i = 0; i < 1000000; i++) {
                for(u32 j = 0; j < 10000000; j++);
            }
        }
    */
    return;
}
