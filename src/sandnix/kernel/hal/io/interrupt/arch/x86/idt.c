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

#include "idt.h"
#include "../../../../early_print/early_print.h"
#include "int_hndlr.h"

static idt_t		idt_table[256];
static void			fill_idt();

void idt_init()
{
    hal_early_print_printf("Initializing IDT...\n");
    fill_idt();

    //Load idt
    idt_reg_t idt_reg = {
        .base = (u32)idt_table,
        .limit = 256 * sizeof(idt_t) - 1
    };

    __asm__ __volatile__(
        "lidt		%0\n\t"
        ::"m"(idt_reg));

    //Set IOPL
    __asm__ __volatile__(
        "pushfl\n\t"
        "movl	(%%esp),%%eax\n\t"
        "andl	$0xFFFFCFFF,%%eax\n\t"
        "movl	%%eax,(%%esp)\n\t"
        "popfl\n\t"
        ::);

    return;
}

void hal_io_int_disable()
{
    __asm__ __volatile__("cli\n");
    return;
}

void hal_io_int_enable()
{
    __asm__ __volatile__("sti\n");
    return;
}

void fill_idt()
{
    SET_NORMAL_IDT(idt_table, 0x00);
    SET_NORMAL_IDT(idt_table, 0x01);
    SET_NORMAL_IDT(idt_table, 0x02);
    SET_IDT(idt_table, 0x03, int_0x03, SELECTOR_K_CODE, TYPE_TRAP, 0, 3, 1);
    SET_NORMAL_IDT(idt_table, 0x03);
    SET_NORMAL_IDT(idt_table, 0x04);
    SET_NORMAL_IDT(idt_table, 0x05);
    SET_NORMAL_IDT(idt_table, 0x06);
    SET_NORMAL_IDT(idt_table, 0x07);
    SET_NORMAL_IDT(idt_table, 0x08);
    SET_NORMAL_IDT(idt_table, 0x09);
    SET_NORMAL_IDT(idt_table, 0x0A);
    SET_NORMAL_IDT(idt_table, 0x0B);
    SET_NORMAL_IDT(idt_table, 0x0C);
    SET_NORMAL_IDT(idt_table, 0x0D);
    SET_NORMAL_IDT(idt_table, 0x0E);
    SET_NORMAL_IDT(idt_table, 0x0F);
    SET_NORMAL_IDT(idt_table, 0x10);
    SET_NORMAL_IDT(idt_table, 0x11);
    SET_NORMAL_IDT(idt_table, 0x12);
    SET_NORMAL_IDT(idt_table, 0x13);
    SET_NORMAL_IDT(idt_table, 0x14);
    SET_NORMAL_IDT(idt_table, 0x15);
    SET_NORMAL_IDT(idt_table, 0x16);
    SET_NORMAL_IDT(idt_table, 0x17);
    SET_NORMAL_IDT(idt_table, 0x18);
    SET_NORMAL_IDT(idt_table, 0x19);
    SET_NORMAL_IDT(idt_table, 0x1A);
    SET_NORMAL_IDT(idt_table, 0x1B);
    SET_NORMAL_IDT(idt_table, 0x1C);
    SET_NORMAL_IDT(idt_table, 0x1D);
    SET_NORMAL_IDT(idt_table, 0x1E);
    SET_NORMAL_IDT(idt_table, 0x1F);
    SET_NORMAL_IDT(idt_table, 0x20);
    SET_NORMAL_IDT(idt_table, 0x21);
    SET_NORMAL_IDT(idt_table, 0x22);
    SET_NORMAL_IDT(idt_table, 0x23);
    SET_NORMAL_IDT(idt_table, 0x24);
    SET_NORMAL_IDT(idt_table, 0x25);
    SET_NORMAL_IDT(idt_table, 0x26);
    SET_NORMAL_IDT(idt_table, 0x27);
    SET_NORMAL_IDT(idt_table, 0x28);
    SET_NORMAL_IDT(idt_table, 0x29);
    SET_NORMAL_IDT(idt_table, 0x2A);
    SET_NORMAL_IDT(idt_table, 0x2B);
    SET_NORMAL_IDT(idt_table, 0x2C);
    SET_NORMAL_IDT(idt_table, 0x2D);
    SET_NORMAL_IDT(idt_table, 0x2E);
    SET_NORMAL_IDT(idt_table, 0x2F);
    SET_NORMAL_IDT(idt_table, 0x30);
    SET_NORMAL_IDT(idt_table, 0x31);
    SET_NORMAL_IDT(idt_table, 0x32);
    SET_NORMAL_IDT(idt_table, 0x33);
    SET_NORMAL_IDT(idt_table, 0x34);
    SET_NORMAL_IDT(idt_table, 0x35);
    SET_NORMAL_IDT(idt_table, 0x36);
    SET_NORMAL_IDT(idt_table, 0x37);
    SET_NORMAL_IDT(idt_table, 0x38);
    SET_NORMAL_IDT(idt_table, 0x39);
    SET_NORMAL_IDT(idt_table, 0x3A);
    SET_NORMAL_IDT(idt_table, 0x3B);
    SET_NORMAL_IDT(idt_table, 0x3C);
    SET_NORMAL_IDT(idt_table, 0x3D);
    SET_NORMAL_IDT(idt_table, 0x3E);
    SET_NORMAL_IDT(idt_table, 0x3F);
    SET_NORMAL_IDT(idt_table, 0x40);
    SET_NORMAL_IDT(idt_table, 0x41);
    SET_NORMAL_IDT(idt_table, 0x42);
    SET_NORMAL_IDT(idt_table, 0x43);
    SET_NORMAL_IDT(idt_table, 0x44);
    SET_NORMAL_IDT(idt_table, 0x45);
    SET_NORMAL_IDT(idt_table, 0x46);
    SET_NORMAL_IDT(idt_table, 0x47);
    SET_NORMAL_IDT(idt_table, 0x48);
    SET_NORMAL_IDT(idt_table, 0x49);
    SET_NORMAL_IDT(idt_table, 0x4A);
    SET_NORMAL_IDT(idt_table, 0x4B);
    SET_NORMAL_IDT(idt_table, 0x4C);
    SET_NORMAL_IDT(idt_table, 0x4D);
    SET_NORMAL_IDT(idt_table, 0x4E);
    SET_NORMAL_IDT(idt_table, 0x4F);
    SET_NORMAL_IDT(idt_table, 0x50);
    SET_NORMAL_IDT(idt_table, 0x51);
    SET_NORMAL_IDT(idt_table, 0x52);
    SET_NORMAL_IDT(idt_table, 0x53);
    SET_NORMAL_IDT(idt_table, 0x54);
    SET_NORMAL_IDT(idt_table, 0x55);
    SET_NORMAL_IDT(idt_table, 0x56);
    SET_NORMAL_IDT(idt_table, 0x57);
    SET_NORMAL_IDT(idt_table, 0x58);
    SET_NORMAL_IDT(idt_table, 0x59);
    SET_NORMAL_IDT(idt_table, 0x5A);
    SET_NORMAL_IDT(idt_table, 0x5B);
    SET_NORMAL_IDT(idt_table, 0x5C);
    SET_NORMAL_IDT(idt_table, 0x5D);
    SET_NORMAL_IDT(idt_table, 0x5E);
    SET_NORMAL_IDT(idt_table, 0x5F);
    SET_NORMAL_IDT(idt_table, 0x60);
    SET_NORMAL_IDT(idt_table, 0x61);
    SET_NORMAL_IDT(idt_table, 0x62);
    SET_NORMAL_IDT(idt_table, 0x63);
    SET_NORMAL_IDT(idt_table, 0x64);
    SET_NORMAL_IDT(idt_table, 0x65);
    SET_NORMAL_IDT(idt_table, 0x66);
    SET_NORMAL_IDT(idt_table, 0x67);
    SET_NORMAL_IDT(idt_table, 0x68);
    SET_NORMAL_IDT(idt_table, 0x69);
    SET_NORMAL_IDT(idt_table, 0x6A);
    SET_NORMAL_IDT(idt_table, 0x6B);
    SET_NORMAL_IDT(idt_table, 0x6C);
    SET_NORMAL_IDT(idt_table, 0x6D);
    SET_NORMAL_IDT(idt_table, 0x6E);
    SET_NORMAL_IDT(idt_table, 0x6F);
    SET_NORMAL_IDT(idt_table, 0x70);
    SET_NORMAL_IDT(idt_table, 0x71);
    SET_NORMAL_IDT(idt_table, 0x72);
    SET_NORMAL_IDT(idt_table, 0x73);
    SET_NORMAL_IDT(idt_table, 0x74);
    SET_NORMAL_IDT(idt_table, 0x75);
    SET_NORMAL_IDT(idt_table, 0x76);
    SET_NORMAL_IDT(idt_table, 0x77);
    SET_NORMAL_IDT(idt_table, 0x78);
    SET_NORMAL_IDT(idt_table, 0x79);
    SET_NORMAL_IDT(idt_table, 0x7A);
    SET_NORMAL_IDT(idt_table, 0x7B);
    SET_NORMAL_IDT(idt_table, 0x7C);
    SET_NORMAL_IDT(idt_table, 0x7D);
    SET_NORMAL_IDT(idt_table, 0x7E);
    SET_NORMAL_IDT(idt_table, 0x7F);
    SET_NORMAL_IDT(idt_table, 0x80);
    SET_NORMAL_IDT(idt_table, 0x81);
    SET_NORMAL_IDT(idt_table, 0x82);
    SET_NORMAL_IDT(idt_table, 0x83);
    SET_NORMAL_IDT(idt_table, 0x84);
    SET_NORMAL_IDT(idt_table, 0x85);
    SET_NORMAL_IDT(idt_table, 0x86);
    SET_NORMAL_IDT(idt_table, 0x87);
    SET_NORMAL_IDT(idt_table, 0x88);
    SET_NORMAL_IDT(idt_table, 0x89);
    SET_NORMAL_IDT(idt_table, 0x8A);
    SET_NORMAL_IDT(idt_table, 0x8B);
    SET_NORMAL_IDT(idt_table, 0x8C);
    SET_NORMAL_IDT(idt_table, 0x8D);
    SET_NORMAL_IDT(idt_table, 0x8E);
    SET_NORMAL_IDT(idt_table, 0x8F);
    SET_NORMAL_IDT(idt_table, 0x90);
    SET_NORMAL_IDT(idt_table, 0x91);
    SET_NORMAL_IDT(idt_table, 0x92);
    SET_NORMAL_IDT(idt_table, 0x93);
    SET_NORMAL_IDT(idt_table, 0x94);
    SET_NORMAL_IDT(idt_table, 0x95);
    SET_NORMAL_IDT(idt_table, 0x96);
    SET_NORMAL_IDT(idt_table, 0x97);
    SET_NORMAL_IDT(idt_table, 0x98);
    SET_NORMAL_IDT(idt_table, 0x99);
    SET_NORMAL_IDT(idt_table, 0x9A);
    SET_NORMAL_IDT(idt_table, 0x9B);
    SET_NORMAL_IDT(idt_table, 0x9C);
    SET_NORMAL_IDT(idt_table, 0x9D);
    SET_NORMAL_IDT(idt_table, 0x9E);
    SET_NORMAL_IDT(idt_table, 0x9F);
    SET_NORMAL_IDT(idt_table, 0xA0);
    SET_NORMAL_IDT(idt_table, 0xA1);
    SET_NORMAL_IDT(idt_table, 0xA2);
    SET_NORMAL_IDT(idt_table, 0xA3);
    SET_NORMAL_IDT(idt_table, 0xA4);
    SET_NORMAL_IDT(idt_table, 0xA5);
    SET_NORMAL_IDT(idt_table, 0xA6);
    SET_NORMAL_IDT(idt_table, 0xA7);
    SET_NORMAL_IDT(idt_table, 0xA8);
    SET_NORMAL_IDT(idt_table, 0xA9);
    SET_NORMAL_IDT(idt_table, 0xAA);
    SET_NORMAL_IDT(idt_table, 0xAB);
    SET_NORMAL_IDT(idt_table, 0xAC);
    SET_NORMAL_IDT(idt_table, 0xAD);
    SET_NORMAL_IDT(idt_table, 0xAE);
    SET_NORMAL_IDT(idt_table, 0xAF);
    SET_NORMAL_IDT(idt_table, 0xB0);
    SET_NORMAL_IDT(idt_table, 0xB1);
    SET_NORMAL_IDT(idt_table, 0xB2);
    SET_NORMAL_IDT(idt_table, 0xB3);
    SET_NORMAL_IDT(idt_table, 0xB4);
    SET_NORMAL_IDT(idt_table, 0xB5);
    SET_NORMAL_IDT(idt_table, 0xB6);
    SET_NORMAL_IDT(idt_table, 0xB7);
    SET_NORMAL_IDT(idt_table, 0xB8);
    SET_NORMAL_IDT(idt_table, 0xB9);
    SET_NORMAL_IDT(idt_table, 0xBA);
    SET_NORMAL_IDT(idt_table, 0xBB);
    SET_NORMAL_IDT(idt_table, 0xBC);
    SET_NORMAL_IDT(idt_table, 0xBD);
    SET_NORMAL_IDT(idt_table, 0xBE);
    SET_NORMAL_IDT(idt_table, 0xBF);
    SET_NORMAL_IDT(idt_table, 0xC0);
    SET_NORMAL_IDT(idt_table, 0xC1);
    SET_NORMAL_IDT(idt_table, 0xC2);
    SET_NORMAL_IDT(idt_table, 0xC3);
    SET_NORMAL_IDT(idt_table, 0xC4);
    SET_NORMAL_IDT(idt_table, 0xC5);
    SET_NORMAL_IDT(idt_table, 0xC6);
    SET_NORMAL_IDT(idt_table, 0xC7);
    SET_NORMAL_IDT(idt_table, 0xC8);
    SET_NORMAL_IDT(idt_table, 0xC9);
    SET_NORMAL_IDT(idt_table, 0xCA);
    SET_NORMAL_IDT(idt_table, 0xCB);
    SET_NORMAL_IDT(idt_table, 0xCC);
    SET_NORMAL_IDT(idt_table, 0xCD);
    SET_NORMAL_IDT(idt_table, 0xCE);
    SET_NORMAL_IDT(idt_table, 0xCF);
    SET_NORMAL_IDT(idt_table, 0xD0);
    SET_NORMAL_IDT(idt_table, 0xD1);
    SET_NORMAL_IDT(idt_table, 0xD2);
    SET_NORMAL_IDT(idt_table, 0xD3);
    SET_NORMAL_IDT(idt_table, 0xD4);
    SET_NORMAL_IDT(idt_table, 0xD5);
    SET_NORMAL_IDT(idt_table, 0xD6);
    SET_NORMAL_IDT(idt_table, 0xD7);
    SET_NORMAL_IDT(idt_table, 0xD8);
    SET_NORMAL_IDT(idt_table, 0xD9);
    SET_NORMAL_IDT(idt_table, 0xDA);
    SET_NORMAL_IDT(idt_table, 0xDB);
    SET_NORMAL_IDT(idt_table, 0xDC);
    SET_NORMAL_IDT(idt_table, 0xDD);
    SET_NORMAL_IDT(idt_table, 0xDE);
    SET_NORMAL_IDT(idt_table, 0xDF);
    SET_NORMAL_IDT(idt_table, 0xE0);
    SET_NORMAL_IDT(idt_table, 0xE1);
    SET_NORMAL_IDT(idt_table, 0xE2);
    SET_NORMAL_IDT(idt_table, 0xE3);
    SET_NORMAL_IDT(idt_table, 0xE4);
    SET_NORMAL_IDT(idt_table, 0xE5);
    SET_NORMAL_IDT(idt_table, 0xE6);
    SET_NORMAL_IDT(idt_table, 0xE7);
    SET_NORMAL_IDT(idt_table, 0xE8);
    SET_NORMAL_IDT(idt_table, 0xE9);
    SET_NORMAL_IDT(idt_table, 0xEA);
    SET_NORMAL_IDT(idt_table, 0xEB);
    SET_NORMAL_IDT(idt_table, 0xEC);
    SET_NORMAL_IDT(idt_table, 0xED);
    SET_NORMAL_IDT(idt_table, 0xEE);
    SET_NORMAL_IDT(idt_table, 0xEF);
    SET_NORMAL_IDT(idt_table, 0xF0);
    SET_NORMAL_IDT(idt_table, 0xF1);
    SET_NORMAL_IDT(idt_table, 0xF2);
    SET_NORMAL_IDT(idt_table, 0xF3);
    SET_NORMAL_IDT(idt_table, 0xF4);
    SET_NORMAL_IDT(idt_table, 0xF5);
    SET_NORMAL_IDT(idt_table, 0xF6);
    SET_NORMAL_IDT(idt_table, 0xF7);
    SET_NORMAL_IDT(idt_table, 0xF8);
    SET_NORMAL_IDT(idt_table, 0xF9);
    SET_NORMAL_IDT(idt_table, 0xFA);
    SET_NORMAL_IDT(idt_table, 0xFB);
    SET_NORMAL_IDT(idt_table, 0xFC);
    SET_NORMAL_IDT(idt_table, 0xFD);
    SET_NORMAL_IDT(idt_table, 0xFE);
    SET_NORMAL_IDT(idt_table, 0xFF);

    return;
}
