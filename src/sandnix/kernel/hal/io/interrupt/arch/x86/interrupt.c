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

#include "../../../../../core/rtl/rtl.h"
#include "../../../../../core/pm/pm.h"
#include "../../../../exception/exception.h"
#include "../../../../early_print/early_print.h"

#include "idt.h"
#include "interrupt.h"
#include "apic.h"

static	spnlck_t			lock;
static	int_callback_t		int_hndlr_table[256] = {NULL};

void interrupt_init()
{
    hal_early_print_printf("Initializing interrupt...\n");
    core_pm_spnlck_init(&lock);
    idt_init();
    tss_init();
    apic_init();
    return;
}

void* hal_io_int_callback_set(u32 num, int_callback_t callback)
{
    core_pm_spnlck_lock(&lock);
    int_callback_t old_hndlr = int_hndlr_table[num];;
    int_hndlr_table[num] = callback;
    core_pm_spnlck_unlock(&lock);
    return old_hndlr;
}

void int_except_dispatcher(u32 int_num, pcontext_t p_context, u32 err_code)
{
    int_callback_t hndlr = int_hndlr_table[int_num];

    if(hndlr != NULL) {
        hndlr(int_num, p_context, err_code);
    }

    PANIC(EINTERRUPT, "Unhandled exception.\n"
          "Interrupt number = 0x%.4X. Error code = %p.\n"
          "Context:\n"
          "EAX = %p.\n"
          "EBX = %p.\n"
          "ECX = %p.\n"
          "EDX = %p.\n"
          "ESI = %p.\n"
          "EDI = %p.\n"
          "ESP = %p.\n"
          "EBP = %p.\n"
          "EIP = %p.\n"
          "EFLAGS = %p.\n"
          "CS = 0x%.4X.\n"
          "SS = 0x%.4X.\n"
          "DS = 0x%.4X.\n"
          "ES = 0x%.4X.\n"
          "FS = 0x%.4X.\n"
          "GS = 0x%.4X.\n",
          int_num, err_code, p_context->eax, p_context->ebx,
          p_context->ecx, p_context->edx, p_context->esi,
          p_context->edi, p_context->esp, p_context->ebp,
          p_context->eip, p_context->eflags, p_context->cs,
          p_context->ss, p_context->ds, p_context->es,
          p_context->fs, p_context->gs);
    return;
}

void int_dispatcher(u32 int_num, pcontext_t p_context)
{
    int_callback_t hndlr = int_hndlr_table[int_num];

    if(hndlr != NULL) {
        hndlr(int_num, p_context, 0);
    }

    if(int_num == INT_IPI) {
        hal_io_IPI_send_eoi();

    } else if(int_num >= REQUIRE_EOI_BEGIN && int_num <= REQUIRE_EOI_END) {
        hal_io_irq_send_eoi();
    }

    hal_cpu_context_load(p_context);
    return;
}
