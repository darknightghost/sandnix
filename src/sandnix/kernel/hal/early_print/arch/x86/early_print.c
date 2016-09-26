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

#include "../../../../core/rtl/rtl.h"
#include "../../../mmu/mmu.h"
#include "../../../../core/pm/pm.h"

#undef	HAL_EARLY_PRINT_EXPORT
#include "../../early_print.h"

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

#define	BASIC_VIDEO_ADDR	(void*)(0x000A0000)

static void*	basic_video_addr = NULL;

static volatile	u32	current_cursor_line;
static volatile	u32	current_cursor_row;

static volatile	u8	bg;
static volatile	u8	fg;

static spnlck_t	lock;

static void			update_cursor();
static void			scoll_down();
static void			out_ch(u16 ch);
static const char*	analyse_color_escape(const char* p);
static bool			get_color_num(const char** p_p, u8* p_num);

void hal_early_print_init()
{
    //Initialize lock
    core_pm_spnlck_init(&lock);

    //Map memory
    if(basic_video_addr == NULL) {
        basic_video_addr = hal_mmu_add_early_paging_addr(BASIC_VIDEO_ADDR,
                           MMU_PAGE_RW);
    }

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

    core_pm_spnlck_lock(&lock);

    current_cursor_line = 0;
    current_cursor_row = 0;
    update_cursor();

    filled_char = ((u16)(bg | fg)) << 8 | ' ';

    //Fill screen
    __asm__ __volatile__(
        "cld\n"
        "rep	stosw\n"
        ::"ax"(filled_char),
        "c"(DEFAULT_STDOUT_WIDTH * DEFAULT_STDOUT_HEIGHT),
        "D"(basic_video_addr)
        :"memory");

    core_pm_spnlck_unlock(&lock);

    return;
}

void hal_early_print_color(u32 new_fg, u32 new_bg)
{
    core_pm_spnlck_lock(&lock);

    fg = new_fg;
    bg = new_bg;

    core_pm_spnlck_unlock(&lock);

    return;
}

void hal_early_print_puts(const char* str)
{
    const char* p;
    u16 ch;
    u32 num;

    core_pm_spnlck_lock(&lock);

    //TODO:Write but

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

                    update_cursor();

                    break;

                case	0x0B:
                case	0x0C:
                    current_cursor_line++;

                    if(current_cursor_line >= DEFAULT_STDOUT_HEIGHT) {
                        u32 old_row;
                        old_row = current_cursor_row;
                        scoll_down();
                        current_cursor_row = old_row;
                    }

                    update_cursor();

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
                    p = analyse_color_escape(p);
                    break;

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

    core_pm_spnlck_unlock(&lock);

    return;
}


void update_cursor()
{
    u16 pos;

    MEM_BLOCK;
    pos = current_cursor_line * DEFAULT_STDOUT_WIDTH + current_cursor_row;

    __asm__ __volatile__(
        "outb	%0, %1\n"
        ::"al"((u8)CURSOR_POS_H_REG), "dx"((u16)(CRTC_ADDR_REG))
        :"memory");

    __asm__ __volatile__(
        "outb	%0, %1\n"
        ::"al"((u8)((pos >> 8) & 0xFF)), "dx"((u16)(CRTC_DATA_REG))
        :"memory");

    __asm__ __volatile__(
        "outb	%0, %1\n"
        ::"al"((u8)(CURSOR_POS_L_REG)), "dx"((u16)(CRTC_ADDR_REG))
        :"memory");

    __asm__ __volatile__(
        "outb	%0, %1\n"
        ::"al"((u8)(pos & 0xFF)), "dx"((u16)(CRTC_DATA_REG))
        :"memory");
    return;
}

void scoll_down()
{
    __asm__ __volatile__(
        "cld\n"
        "rep	movsw\n"
        ::"c"(DEFAULT_STDOUT_WIDTH * (DEFAULT_STDOUT_HEIGHT - 1)),
        "S"((address_t)basic_video_addr + DEFAULT_STDOUT_WIDTH * sizeof(u16)),
        "D"(basic_video_addr)
        : "memory");

    __asm__ __volatile__(
        "cld\n"
        "rep	stosw\n"
        ::"a"(((u16)(bg | fg)) << 8 | ' '),
        "c"(DEFAULT_STDOUT_WIDTH), "D"((address_t)basic_video_addr +
                                       DEFAULT_STDOUT_WIDTH * (DEFAULT_STDOUT_HEIGHT - 1) * 2)
        :"memory");


    current_cursor_line--;
    MEM_BLOCK;
    update_cursor();
    return;
}

void out_ch(u16 ch)
{
    *((u16*)basic_video_addr + current_cursor_line * DEFAULT_STDOUT_WIDTH
      + current_cursor_row) = ch;
    current_cursor_row++;

    if(current_cursor_row >= DEFAULT_STDOUT_WIDTH) {
        current_cursor_row = 0;
        current_cursor_line++;
    }

    if(current_cursor_line >= DEFAULT_STDOUT_HEIGHT) {
        scoll_down();
    }

    update_cursor();

    return;
}

const char* analyse_color_escape(const char* p)
{
    u8 new_bg = 0;
    u8 new_fg = 70;

    p++;

    if(*p != '[') {
        return p;
    }

    p++;

    u8 num;

    //Analyse parameters
    while(get_color_num(&p, &num)) {
        if(num >= 30 && num < 40) {
            //Foreground
            switch(*p) {
                case 30:
                    //Black
                    new_fg = (new_fg & 0x80) | 0x00;
                    break;

                case 31:
                    //Red
                    new_fg = (new_fg & 0x80) | 0x40;
                    break;

                case 32:
                    //Green
                    new_fg = (new_fg & 0x80) | 0x20;
                    break;

                case 33:
                    //Yellow
                    new_fg = (new_fg & 0x80) | 0x60;
                    break;

                case 34:
                    //Blue
                    new_fg = (new_fg & 0x80) | 0x10;
                    break;

                case 35:
                    //Magenta
                    new_fg = (new_fg & 0x80) | 0x50;
                    break;

                case 36:
                    //Cyan
                    new_fg = (new_fg & 0x80) | 0x30;
                    break;

                case 37:
                    //White
                    new_fg = (new_fg & 0x80) | 0x70;
                    break;
            }

        } else if(num >= 40 && num < 50) {
            //Background
            switch(*p) {
                case 40:
                    //Black
                    new_bg = (new_bg & 0x08) | 0x00;
                    break;

                case 41:
                    //Red
                    new_bg = (new_bg & 0x08) | 0x04;
                    break;

                case 42:
                    //Green
                    new_bg = (new_bg & 0x08) | 0x02;
                    break;

                case 43:
                    //Yellow
                    new_bg = (new_bg & 0x08) | 0x06;
                    break;

                case 44:
                    //Blue
                    new_bg = (new_bg & 0x08) | 0x01;
                    break;

                case 45:
                    //Magenta
                    new_bg = (new_bg & 0x08) | 0x05;
                    break;

                case 46:
                    //Cyan
                    new_bg = (new_bg & 0x08) | 0x03;
                    break;

                case 47:
                    //White
                    new_bg = (new_bg & 0x08) | 0x07;
                    break;
            }

        } else {
            switch(*p) {
                case 5:
                    //Blink
                    new_bg = new_bg | 0x08;
                    break;

                case 25:
                    //Not blink
                    new_bg = new_bg & 0x07;
                    break;

                case 1:
                    //High light
                    new_fg = new_fg | 0x80;
                    break;
            }
        }
    }

    if(*p == 'm') {
        bg = new_bg;
        fg = new_fg;
    }

    p++;

    return p;
}

bool get_color_num(const char** p_p, u8* p_num)
{
    if(**p_p > '9' &&**p_p < '0') {
        return false;
    }

    *p_num = (u8)core_rtl_atoi((char*)(*p_p), 10);

    while(**p_p >= '0' &&**p_p <= '9') {
        (*p_p)++;
    }

    if(**p_p == ';') {
        (*p_p)++;
    }

    return true;
}
