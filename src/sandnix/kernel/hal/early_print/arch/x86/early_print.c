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

#include "../../early_print.h"
#include "../../../mmu/mmu.h"
#include "../../../../core/pm/pm.h"

#define	DEFAULT_STDOUT_WIDTH	80
#define	DEFAULT_STDOUT_HEIGHT	25


#define	CRTC_ADDR_REG		0x03D4
#define	CRTC_DATA_REG		0x03D5
#define	GC_ADDR_REG			0x03CE
#define	GC_DATA_REG			0x03CF
#define	CURSOR_POS_H_REG	0x0E
#define	CURSOR_POS_L_REG	0x0F
#define	START_ADDR_H_REG	0x0C
#define	START_ADDR_L_REG	0x0D

#define	BASIC_VIDEO_ADDR	(0x000A0000 + KERNEL_MEM_BASE)

static volatile	u32	current_cursor_line;
static volatile	u32	current_cursor_row;

static volatile	u8	bg;
static volatile	u8	fg;

static spnlck_t	lock;

static void		update_cursor();
static void		scoll_down();
static void		out_ch(u16 ch);

void hal_early_print_init()
{
    //Initialize lock
    core_pm_spnlck_init(&lock);

    //Set video card to VGA text mode.
    __asm__ __volatile__(
        "movb	$0x06, %%al\n"
        "movw	%0, %%dx\n"
        "outb	%%al, %%dx\n"
        "xorl	%%eax, %%eax\n"
        "movw	%1, %%dx\n"
        "outb	%%al, %%dx\n"
        ::"i"((u16)(GC_ADDR_REG)), "i"((u16)GC_DATA_REG)
        :"eax", "edx", "memory");

    //Set screen address
    __asm__ __volatile__(
        "movb	%2, %%al\n"
        "movw	%0, %%dx\n"
        "outb	%%al, %%dx\n"
        "xorl	%%eax, %%eax\n"
        "movw	%1, %%dx\n"
        "outb	%%al, %%dx\n"
        "movb	%3, %%al\n"
        "movw	%0, %%dx\n"
        "outb	%%al, %%dx\n"
        "xorl	%%eax, %%eax\n"
        "movw	%1, %%dx\n"
        "outb	%%al, %%dx\n"
        ::"i"((u16)(CRTC_ADDR_REG)), "i"((u16)(CRTC_DATA_REG)),
        "i"((u8)(START_ADDR_H_REG)), "i"((u8)(START_ADDR_L_REG))
        :"eax", "memory");

    //Set bachground color
    hal_early_print_color(FG_BRIGHT_WHITE, BG_BLACK);

    //Clear screen
    hal_early_print_cls();

    return;
}

void hal_early_print_cls()
{
    u16 filled_char;

    core_pm_spnlck_raw_lock(&lock);

    current_cursor_line = 0;
    current_cursor_row = 0;
    update_cursor();

    filled_char = ((u16)(bg | fg)) << 8 | ' ';

    //Fill screen
    __asm__ __volatile__(
        "cld\n"
        "rep	stosw\n"
        ::"ax"(filled_char),
        "ecx"(DEFAULT_STDOUT_WIDTH * DEFAULT_STDOUT_HEIGHT),
        "edi"(BASIC_VIDEO_ADDR)
        :"memory");

    core_pm_spnlck_raw_unlock(&lock);

    return;
}

void hal_early_print_color(u32 new_fg, u32 new_bg)
{
    core_pm_spnlck_raw_lock(&lock);

    fg = new_fg;
    bg = new_bg;

    core_pm_spnlck_raw_unlock(&lock);

    return;
}

void hal_early_print_puts(char* str)
{
    char* p;
    u16 ch;
    u32 num;

    core_pm_spnlck_raw_lock(&lock);

    for(p = str; *p != '\0'; p++) {
        if(*p < 0x20) {
            //Control character
            switch(*p) {
                case	0x00:
                case	0x01:
                case	0x02:
                case	0x03:
                case	0x04:
                case	0x05:
                case	0x06:
                    break;

                case	0x07:
                    //Bell
                    break;

                case	0x08:
                    if(current_cursor_row > 0) {
                        current_cursor_row--;
                        update_cursor();
                    }

                    break;

                case	0x09:
                    for(num = 8 - current_cursor_row % 8;
                        num > 0;
                        num--) {
                        ch = ((u16)(bg | fg)) << 8 | ' ';
                        out_ch(ch);
                    }

                    break;

                case	0x0A:
                    current_cursor_line++;
                    current_cursor_row = 0;

                    if(current_cursor_line >= DEFAULT_STDOUT_HEIGHT) {
                        scoll_down();
                    }

                    break;

                case	0x0B:
                case	0x0C:
                    current_cursor_line++;

                    if(current_cursor_line >= DEFAULT_STDOUT_HEIGHT) {
                        u32 old_row;
                        old_row = current_cursor_row;
                        scoll_down();
                        current_cursor_row = old_row;
                        update_cursor();
                    }

                    break;

                case	0x0D:
                    current_cursor_row = 0;
                    update_cursor();

                case	0x0E:
                case	0x0F:
                case	0x10:
                case	0x11:
                case	0x12:
                case	0x13:
                case	0x14:
                case	0x15:
                case	0x16:
                case	0x17:
                case	0x18:
                case	0x19:
                case	0x1A:
                case	0x1B:
                case	0x1C:
                case	0x1D:
                case	0x1E:
                case	0x1F:
                    break;
            }

        } else if(*p < 0x7F) {
            //Printable character
            ch = ((u16)(bg | fg)) << 8 | *p;
            out_ch(ch);

        } else if(*p == 0x7F) {
            continue;

        } else {
            ch = ((u16)(bg | fg)) << 8 | '?';
            out_ch(ch);
        }
    }

    core_pm_spnlck_raw_unlock(&lock);

    return;
}


void update_cursor()
{
    u16 pos;

    pos = current_cursor_line * DEFAULT_STDOUT_WIDTH + current_cursor_row;

    __asm__ __volatile__(
        "outb	%%al, %1\n"
        ::"al"((u8)CURSOR_POS_H_REG), "dx"((u16)(CRTC_ADDR_REG))
        :"memory");

    __asm__ __volatile__(
        "outb	%%al, %1\n"
        ::"al"((u8)((pos >> 8) & 0xFF)), "dx"((u16)(CRTC_DATA_REG))
        :"memory");

    __asm__ __volatile__(
        "outb	%%al, %1\n"
        ::"al"((u8)(CURSOR_POS_L_REG)), "dx"((u16)(CRTC_ADDR_REG))
        :"memory");

    __asm__ __volatile__(
        "outb	%%al, %1\n"
        ::"al"((u8)(pos & 0xFF)), "dx"((u16)(CRTC_DATA_REG))
        :"memory");
    return;
}

void scoll_down()
{
    u16 filled_char;

    //Scoll
    __asm__ __volatile__(
        "cld\n"
        "rep	movsw\n"
        ::"ecx"(DEFAULT_STDOUT_WIDTH * (DEFAULT_STDOUT_HEIGHT - 1)),
        "esi"((address_t)BASIC_VIDEO_ADDR + DEFAULT_STDOUT_WIDTH * sizeof(u16)),
        "edi"(BASIC_VIDEO_ADDR)
        :"memory");

    //Clear last line
    filled_char = ((u16)(bg | fg)) << 8 | ' ';
    __asm__ __volatile__(
        "cld\n"
        "rep	stosw\n"
        ::"ax"(filled_char),
        "ecx"(DEFAULT_STDOUT_WIDTH),
        "edi"((address_t)BASIC_VIDEO_ADDR +
              DEFAULT_STDOUT_WIDTH * (DEFAULT_STDOUT_HEIGHT - 1) * 2)
        :"memory");

    current_cursor_line--;
    update_cursor();
    return;
}

void out_ch(u16 ch)
{
    *((u16*)BASIC_VIDEO_ADDR + current_cursor_line * current_cursor_row) = ch;
    current_cursor_row++;

    if(current_cursor_row >= DEFAULT_STDOUT_WIDTH) {
        current_cursor_row = 0;
        current_cursor_line++;
    }

    if(current_cursor_line >= DEFAULT_STDOUT_HEIGHT) {
        scoll_down();
    }

    return;
}
