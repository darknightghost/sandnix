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


#include "interrupt.h"
#include "ivt.h"
#include "../../../../../../exception/exception.h"
#include "../../../../../../early_print/early_print.h"
#include "../../../../../../../core/pm/pm.h"

static	spnlck_t			lock;
static	int_callback_t		int_hndlr_table[256] = {NULL};

void interrupt_init()
{
    hal_early_print_printf("Initializing interrupt...\n");
    core_pm_spnlck_init(&lock);
    ivt_init();
    gic_init();
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

void hal_io_int_disable()
{
    __asm__ __volatile__(
        "mrs	r0, cpsr\n"
        "orr	r0, r0, %0\n"
        "msr	cpsr, r0\n"
        ::"i"(0xC0)
        :"r0");
    return;
}

void hal_io_int_enable()
{
    __asm__ __volatile__(
        "mrs	r0, cpsr\n"
        "and	r0, r0, %0\n"
        "msr	cpsr, r0\n"
        ::"i"(~(u32)0xC0)
        :"r0");
    return;
}

void reset_hndlr()
{
    PANIC(ENOTSUP, "Reset.");
}

void undef_hndlr(pcontext_t p_context)
{
    int_callback_t callback = int_hndlr_table[INT_UNDEF];

    if(callback != NULL) {
        callback(INT_UNDEF,
                 p_context, 0);
    }

    PANIC(EINTERRUPT, "Unhandled undefined exception.\n"
          "Context:\n"
          "R0 = %p.\n"
          "R1 = %p.\n"
          "R2 = %p.\n"
          "R3 = %p.\n"
          "R4 = %p.\n"
          "R5 = %p.\n"
          "R6 = %p.\n"
          "R7 = %p.\n"
          "R8 = %p.\n"
          "R9 = %p.\n"
          "R10 = %p.\n"
          "R11 = %p.\n"
          "R12 = %p.\n"
          "SP_SVC = %p.\n"
          "LR_SVC = %p.\n"
          "SP_USR = %p.\n"
          "LR_USR = %p.\n"
          "CPSR = %p.\n"
          "PC = %p.\n",
          p_context->r0, p_context->r1, p_context->r2, p_context->r3, p_context->r4,
          p_context->r5, p_context->r6, p_context->r7, p_context->r8, p_context->r9,
          p_context->r10, p_context->r11, p_context->r12, p_context->sp_svc,
          p_context->lr_svc, p_context->sp_usr, p_context->lr_usr, p_context->cpsr,
          p_context->pc);
    return;
}

void swi_hndlr(pcontext_t p_context)
{
    int_callback_t callback = int_hndlr_table[INT_GATE];

    if(callback != NULL) {
        callback(INT_GATE,
                 p_context, 0);
    }

    hal_cpu_context_load(p_context);
}

void prefetch_abort_hndlr(pcontext_t p_context)
{
    int_callback_t callback = int_hndlr_table[INT_PREFETCH_ABT];

    if(callback != NULL) {
        callback(INT_PREFETCH_ABT,
                 p_context, 0);
    }

    PANIC(EINTERRUPT, "Unhandled prefetch abort exception.\n"
          "Context:\n"
          "R0 = %p.\n"
          "R1 = %p.\n"
          "R2 = %p.\n"
          "R3 = %p.\n"
          "R4 = %p.\n"
          "R5 = %p.\n"
          "R6 = %p.\n"
          "R7 = %p.\n"
          "R8 = %p.\n"
          "R9 = %p.\n"
          "R10 = %p.\n"
          "R11 = %p.\n"
          "R12 = %p.\n"
          "SP_SVC = %p.\n"
          "LR_SVC = %p.\n"
          "SP_USR = %p.\n"
          "LR_USR = %p.\n"
          "CPSR = %p.\n"
          "PC = %p.\n",
          p_context->r0, p_context->r1, p_context->r2, p_context->r3, p_context->r4,
          p_context->r5, p_context->r6, p_context->r7, p_context->r8, p_context->r9,
          p_context->r10, p_context->r11, p_context->r12, p_context->sp_svc,
          p_context->lr_svc, p_context->sp_usr, p_context->lr_usr, p_context->cpsr,
          p_context->pc);

    hal_cpu_context_load(p_context);
}

void data_abort_hndlr(pcontext_t p_context)
{
    int_callback_t callback = int_hndlr_table[INT_DATA_ABT];

    if(callback != NULL) {
        callback(INT_DATA_ABT,
                 p_context, 0);
    }

    PANIC(EINTERRUPT, "Unhandled data abort exception.\n"
          "Context:\n"
          "R0 = %p.\n"
          "R1 = %p.\n"
          "R2 = %p.\n"
          "R3 = %p.\n"
          "R4 = %p.\n"
          "R5 = %p.\n"
          "R6 = %p.\n"
          "R7 = %p.\n"
          "R8 = %p.\n"
          "R9 = %p.\n"
          "R10 = %p.\n"
          "R11 = %p.\n"
          "R12 = %p.\n"
          "SP_SVC = %p.\n"
          "LR_SVC = %p.\n"
          "SP_USR = %p.\n"
          "LR_USR = %p.\n"
          "CPSR = %p.\n"
          "PC = %p.\n",
          p_context->r0, p_context->r1, p_context->r2, p_context->r3, p_context->r4,
          p_context->r5, p_context->r6, p_context->r7, p_context->r8, p_context->r9,
          p_context->r10, p_context->r11, p_context->r12, p_context->sp_svc,
          p_context->lr_svc, p_context->sp_usr, p_context->lr_usr, p_context->cpsr,
          p_context->pc);

    hal_cpu_context_load(p_context);
}

void reserved_hndlr()
{
    PANIC(ENOTSUP,
          "Reserved interrupt called.");
    return;
}

void irq_hndlr(pcontext_t p_context)
{
    u32 int_id = gic_get_irq_num();
    int_callback_t callback = int_hndlr_table[int_id + IRQ_BASE];

    if(callback != NULL) {
        callback(int_id + IRQ_BASE, p_context, 0);
    }

    hal_io_irq_send_eoi();
    hal_cpu_context_load(p_context);
    return;
}
