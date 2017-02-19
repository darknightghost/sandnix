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

#define	MODULE_NAME hal_io

static	spnlck_t			lock;
static	int_callback_t		int_hndlr_table[256] = {NULL};

void PRIVATE(interrupt_init)()
{
    hal_early_print_printf("Initializing interrupt...\n");
    core_pm_spnlck_init(&lock);
    PRIVATE(idt_init)();
    PRIVATE(tss_init)();
    PRIVATE(apic_init)();
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

void hal_io_int(u32 int_num)
{
    switch(int_num) {
#define TEST_CALL_INT(n)	case (n) : \
        __asm__ __volatile__( \
                              "int	%0\n" \
                              ::"i"((n)) \
                              :); \
        break;

            TEST_CALL_INT(0);
            TEST_CALL_INT(1);
            TEST_CALL_INT(2);
            TEST_CALL_INT(3);
            TEST_CALL_INT(4);
            TEST_CALL_INT(5);
            TEST_CALL_INT(6);
            TEST_CALL_INT(7);
            TEST_CALL_INT(8);
            TEST_CALL_INT(9);
            TEST_CALL_INT(10);
            TEST_CALL_INT(11);
            TEST_CALL_INT(12);
            TEST_CALL_INT(13);
            TEST_CALL_INT(14);
            TEST_CALL_INT(15);
            TEST_CALL_INT(16);
            TEST_CALL_INT(17);
            TEST_CALL_INT(18);
            TEST_CALL_INT(19);
            TEST_CALL_INT(20);
            TEST_CALL_INT(21);
            TEST_CALL_INT(22);
            TEST_CALL_INT(23);
            TEST_CALL_INT(24);
            TEST_CALL_INT(25);
            TEST_CALL_INT(26);
            TEST_CALL_INT(27);
            TEST_CALL_INT(28);
            TEST_CALL_INT(29);
            TEST_CALL_INT(30);
            TEST_CALL_INT(31);
            TEST_CALL_INT(32);
            TEST_CALL_INT(33);
            TEST_CALL_INT(34);
            TEST_CALL_INT(35);
            TEST_CALL_INT(36);
            TEST_CALL_INT(37);
            TEST_CALL_INT(38);
            TEST_CALL_INT(39);
            TEST_CALL_INT(40);
            TEST_CALL_INT(41);
            TEST_CALL_INT(42);
            TEST_CALL_INT(43);
            TEST_CALL_INT(44);
            TEST_CALL_INT(45);
            TEST_CALL_INT(46);
            TEST_CALL_INT(47);
            TEST_CALL_INT(48);
            TEST_CALL_INT(49);
            TEST_CALL_INT(50);
            TEST_CALL_INT(51);
            TEST_CALL_INT(52);
            TEST_CALL_INT(53);
            TEST_CALL_INT(54);
            TEST_CALL_INT(55);
            TEST_CALL_INT(56);
            TEST_CALL_INT(57);
            TEST_CALL_INT(58);
            TEST_CALL_INT(59);
            TEST_CALL_INT(60);
            TEST_CALL_INT(61);
            TEST_CALL_INT(62);
            TEST_CALL_INT(63);
            TEST_CALL_INT(64);
            TEST_CALL_INT(65);
            TEST_CALL_INT(66);
            TEST_CALL_INT(67);
            TEST_CALL_INT(68);
            TEST_CALL_INT(69);
            TEST_CALL_INT(70);
            TEST_CALL_INT(71);
            TEST_CALL_INT(72);
            TEST_CALL_INT(73);
            TEST_CALL_INT(74);
            TEST_CALL_INT(75);
            TEST_CALL_INT(76);
            TEST_CALL_INT(77);
            TEST_CALL_INT(78);
            TEST_CALL_INT(79);
            TEST_CALL_INT(80);
            TEST_CALL_INT(81);
            TEST_CALL_INT(82);
            TEST_CALL_INT(83);
            TEST_CALL_INT(84);
            TEST_CALL_INT(85);
            TEST_CALL_INT(86);
            TEST_CALL_INT(87);
            TEST_CALL_INT(88);
            TEST_CALL_INT(89);
            TEST_CALL_INT(90);
            TEST_CALL_INT(91);
            TEST_CALL_INT(92);
            TEST_CALL_INT(93);
            TEST_CALL_INT(94);
            TEST_CALL_INT(95);
            TEST_CALL_INT(96);
            TEST_CALL_INT(97);
            TEST_CALL_INT(98);
            TEST_CALL_INT(99);
            TEST_CALL_INT(100);
            TEST_CALL_INT(101);
            TEST_CALL_INT(102);
            TEST_CALL_INT(103);
            TEST_CALL_INT(104);
            TEST_CALL_INT(105);
            TEST_CALL_INT(106);
            TEST_CALL_INT(107);
            TEST_CALL_INT(108);
            TEST_CALL_INT(109);
            TEST_CALL_INT(110);
            TEST_CALL_INT(111);
            TEST_CALL_INT(112);
            TEST_CALL_INT(113);
            TEST_CALL_INT(114);
            TEST_CALL_INT(115);
            TEST_CALL_INT(116);
            TEST_CALL_INT(117);
            TEST_CALL_INT(118);
            TEST_CALL_INT(119);
            TEST_CALL_INT(120);
            TEST_CALL_INT(121);
            TEST_CALL_INT(122);
            TEST_CALL_INT(123);
            TEST_CALL_INT(124);
            TEST_CALL_INT(125);
            TEST_CALL_INT(126);
            TEST_CALL_INT(127);
            TEST_CALL_INT(128);
            TEST_CALL_INT(129);
            TEST_CALL_INT(130);
            TEST_CALL_INT(131);
            TEST_CALL_INT(132);
            TEST_CALL_INT(133);
            TEST_CALL_INT(134);
            TEST_CALL_INT(135);
            TEST_CALL_INT(136);
            TEST_CALL_INT(137);
            TEST_CALL_INT(138);
            TEST_CALL_INT(139);
            TEST_CALL_INT(140);
            TEST_CALL_INT(141);
            TEST_CALL_INT(142);
            TEST_CALL_INT(143);
            TEST_CALL_INT(144);
            TEST_CALL_INT(145);
            TEST_CALL_INT(146);
            TEST_CALL_INT(147);
            TEST_CALL_INT(148);
            TEST_CALL_INT(149);
            TEST_CALL_INT(150);
            TEST_CALL_INT(151);
            TEST_CALL_INT(152);
            TEST_CALL_INT(153);
            TEST_CALL_INT(154);
            TEST_CALL_INT(155);
            TEST_CALL_INT(156);
            TEST_CALL_INT(157);
            TEST_CALL_INT(158);
            TEST_CALL_INT(159);
            TEST_CALL_INT(160);
            TEST_CALL_INT(161);
            TEST_CALL_INT(162);
            TEST_CALL_INT(163);
            TEST_CALL_INT(164);
            TEST_CALL_INT(165);
            TEST_CALL_INT(166);
            TEST_CALL_INT(167);
            TEST_CALL_INT(168);
            TEST_CALL_INT(169);
            TEST_CALL_INT(170);
            TEST_CALL_INT(171);
            TEST_CALL_INT(172);
            TEST_CALL_INT(173);
            TEST_CALL_INT(174);
            TEST_CALL_INT(175);
            TEST_CALL_INT(176);
            TEST_CALL_INT(177);
            TEST_CALL_INT(178);
            TEST_CALL_INT(179);
            TEST_CALL_INT(180);
            TEST_CALL_INT(181);
            TEST_CALL_INT(182);
            TEST_CALL_INT(183);
            TEST_CALL_INT(184);
            TEST_CALL_INT(185);
            TEST_CALL_INT(186);
            TEST_CALL_INT(187);
            TEST_CALL_INT(188);
            TEST_CALL_INT(189);
            TEST_CALL_INT(190);
            TEST_CALL_INT(191);
            TEST_CALL_INT(192);
            TEST_CALL_INT(193);
            TEST_CALL_INT(194);
            TEST_CALL_INT(195);
            TEST_CALL_INT(196);
            TEST_CALL_INT(197);
            TEST_CALL_INT(198);
            TEST_CALL_INT(199);
            TEST_CALL_INT(200);
            TEST_CALL_INT(201);
            TEST_CALL_INT(202);
            TEST_CALL_INT(203);
            TEST_CALL_INT(204);
            TEST_CALL_INT(205);
            TEST_CALL_INT(206);
            TEST_CALL_INT(207);
            TEST_CALL_INT(208);
            TEST_CALL_INT(209);
            TEST_CALL_INT(210);
            TEST_CALL_INT(211);
            TEST_CALL_INT(212);
            TEST_CALL_INT(213);
            TEST_CALL_INT(214);
            TEST_CALL_INT(215);
            TEST_CALL_INT(216);
            TEST_CALL_INT(217);
            TEST_CALL_INT(218);
            TEST_CALL_INT(219);
            TEST_CALL_INT(220);
            TEST_CALL_INT(221);
            TEST_CALL_INT(222);
            TEST_CALL_INT(223);
            TEST_CALL_INT(224);
            TEST_CALL_INT(225);
            TEST_CALL_INT(226);
            TEST_CALL_INT(227);
            TEST_CALL_INT(228);
            TEST_CALL_INT(229);
            TEST_CALL_INT(230);
            TEST_CALL_INT(231);
            TEST_CALL_INT(232);
            TEST_CALL_INT(233);
            TEST_CALL_INT(234);
            TEST_CALL_INT(235);
            TEST_CALL_INT(236);
            TEST_CALL_INT(237);
            TEST_CALL_INT(238);
            TEST_CALL_INT(239);
            TEST_CALL_INT(240);
            TEST_CALL_INT(241);
            TEST_CALL_INT(242);
            TEST_CALL_INT(243);
            TEST_CALL_INT(244);
            TEST_CALL_INT(245);
            TEST_CALL_INT(246);
            TEST_CALL_INT(247);
            TEST_CALL_INT(248);
            TEST_CALL_INT(249);
            TEST_CALL_INT(250);
            TEST_CALL_INT(251);
            TEST_CALL_INT(252);
            TEST_CALL_INT(253);
            TEST_CALL_INT(254);
            TEST_CALL_INT(255);

#undef	TEST_CALL_INT
    }

    return;
}

void int_except_dispatcher(u32 int_num, pcontext_t p_context, u32 err_code)
{
    int_callback_t hndlr = int_hndlr_table[int_num];

    if(hndlr != NULL) {
        hndlr(int_num, p_context, err_code);
    }

    switch(int_num) {
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
                ppf_errcode_t p_errcode = (ppf_errcode_t)(&err_code);

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
