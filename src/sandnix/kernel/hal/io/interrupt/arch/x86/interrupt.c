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
#include "../../../../../core/exception/exception.h"
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

    switch(err_code) {
        case 0x00:
            //DE
            {
                pediv_except_t p_except = ediv_except();
                p_except->except.raise((pexcept_obj_t)p_except,
                                       p_context,
                                       __FILE__,
                                       __LINE__,
                                       NULL);
            }
            break;

        case 0x01:
            //DB
            {
                peunknowint_except_t p_except = eunknowint_except(0x01,
                                                err_code);
                p_except->except.raise((pexcept_obj_t)p_except,
                                       p_context,
                                       __FILE__,
                                       __LINE__,
                                       NULL);
            }
            break;

        case 0x02:
            //NMI
            {
                peunknowint_except_t p_except = eunknowint_except(0x02,
                                                err_code);
                p_except->except.raise((pexcept_obj_t)p_except,
                                       p_context,
                                       __FILE__,
                                       __LINE__,
                                       NULL);
            }
            break;

        case 0x03:
            //BP
            {
                pebreakpoint_except_t p_except = ebreakpoint_except();
                p_except->except.raise((pexcept_obj_t)p_except,
                                       p_context,
                                       __FILE__,
                                       __LINE__,
                                       NULL);
            }
            break;

        case 0x04:
            //OF
            {
                peunknowint_except_t p_except = eunknowint_except(0x02,
                                                err_code);
                p_except->except.raise((pexcept_obj_t)p_except,
                                       p_context,
                                       __FILE__,
                                       __LINE__,
                                       NULL);
            }
            break;

        case 0x05:
            //BR
            {
                peunknowint_except_t p_except = eunknowint_except(0x05,
                                                err_code);
                p_except->except.raise((pexcept_obj_t)p_except,
                                       p_context,
                                       __FILE__,
                                       __LINE__,
                                       NULL);
            }
            break;

        case 0x06:
            //UD
            {
                peundefined_except_t p_except = eundefined_except();
                p_except->except.raise((pexcept_obj_t)p_except,
                                       p_context,
                                       __FILE__,
                                       __LINE__,
                                       NULL);
            }
            break;

        case 0x07:
            //NM
            {
                pefloat_except_t p_except = efloat_except();
                p_except->except.raise((pexcept_obj_t)p_except,
                                       p_context,
                                       __FILE__,
                                       __LINE__,
                                       NULL);
            }
            break;

        case 0x08:
            //DF
            {
                peunknowint_except_t p_except = eunknowint_except(0x08,
                                                err_code);
                p_except->except.raise((pexcept_obj_t)p_except,
                                       p_context,
                                       __FILE__,
                                       __LINE__,
                                       NULL);
            }
            break;

        case 0x09:
            //Reserve(Coprocessor Segment overrun)
            {
                pefloat_except_t p_except = efloat_except();
                p_except->except.raise((pexcept_obj_t)p_except,
                                       p_context,
                                       __FILE__,
                                       __LINE__,
                                       NULL);
            }
            break;

        case 0x0A:
            //TS
            {
                peunknowint_except_t p_except = eunknowint_except(0x0A,
                                                err_code);
                p_except->except.raise((pexcept_obj_t)p_except,
                                       p_context,
                                       __FILE__,
                                       __LINE__,
                                       NULL);
            }
            break;

        case 0x0B:
            //NP
            {
                peunknowint_except_t p_except = eunknowint_except(0x0B,
                                                err_code);
                p_except->except.raise((pexcept_obj_t)p_except,
                                       p_context,
                                       __FILE__,
                                       __LINE__,
                                       NULL);
            }
            break;

        case 0x0C:
            //SS
            {
                peunknowint_except_t p_except = eunknowint_except(0x0C,
                                                err_code);
                p_except->except.raise((pexcept_obj_t)p_except,
                                       p_context,
                                       __FILE__,
                                       __LINE__,
                                       NULL);
            }
            break;

        case 0x0D:
            //GP
            {
                peprivilege_except_t p_except = eprivilege_except();
                p_except->except.raise((pexcept_obj_t)p_except,
                                       p_context,
                                       __FILE__,
                                       __LINE__,
                                       NULL);
            }
            break;

        case 0x0E:
            //PF
            {
                //Get address
                address_t cr2;
                __asm__ __volatile__(
                    "movl	%%cr2, %0\n"
                    :"=r"(cr2)
                    ::);

                //Get reason
                ppf_errcode_t p_errcode = (ppf_errcode_t)(err_code);

                if(p_errcode->bits.w_r) {
                    //Write fault
                    pepagewrite_except_t p_except = epagewrite_except(cr2);
                    p_except->except.raise((pexcept_obj_t)p_except,
                                           p_context,
                                           __FILE__,
                                           __LINE__,
                                           NULL);

                } else {
                    //Read fault
                    pepageread_except_t p_except = epageread_except(cr2);
                    p_except->except.raise((pexcept_obj_t)p_except,
                                           p_context,
                                           __FILE__,
                                           __LINE__,
                                           NULL);
                }
            }
            break;

        case 0x0F:
            //Reserved
            {
                peunknowint_except_t p_except = eunknowint_except(0x0F,
                                                err_code);
                p_except->except.raise((pexcept_obj_t)p_except,
                                       p_context,
                                       __FILE__,
                                       __LINE__,
                                       NULL);
            }
            break;

        case 0x10:
            //MF
            {
                pefloat_except_t p_except = efloat_except();
                p_except->except.raise((pexcept_obj_t)p_except,
                                       p_context,
                                       __FILE__,
                                       __LINE__,
                                       NULL);
            }
            break;

        case 0x11:
            //AC
            {
                peunknowint_except_t p_except = eunknowint_except(0x11,
                                                err_code);
                p_except->except.raise((pexcept_obj_t)p_except,
                                       p_context,
                                       __FILE__,
                                       __LINE__,
                                       NULL);
            }
            break;

        case 0x12:
            //MC
            {
                peunknowint_except_t p_except = eunknowint_except(0x12,
                                                err_code);
                p_except->except.raise((pexcept_obj_t)p_except,
                                       p_context,
                                       __FILE__,
                                       __LINE__,
                                       NULL);
            }
            break;

        case 0x13:
            //XM
            {
                peunknowint_except_t p_except = eunknowint_except(0x13,
                                                err_code);
                p_except->except.raise((pexcept_obj_t)p_except,
                                       p_context,
                                       __FILE__,
                                       __LINE__,
                                       NULL);
            }
            break;
    }

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
