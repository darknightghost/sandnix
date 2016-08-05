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

#include "early_print.h"
#include "../../../../../early_print.h"
#include "../../../../../../mmu/mmu.h"
#include "../../../../../../../core/pm/pm.h"
#include "../../../../../../../core/rtl/rtl.h"

#define	DEFAULT_STDOUT_WIDTH	80
#define	DEFAULT_STDOUT_HEIGHT	25

//Serial reisters
#define	UART2_PHYBASE		((void*)0x13820000)
static	address_t			uart2_base = (address_t)NULL;
#define	ULCON2				(*((volatile u32*)(uart2_base + 0x0000)))
#define	UCON2				(*((volatile u32*)(uart2_base + 0x0004)))
#define	UFCON2				(*((volatile u32*)(uart2_base + 0x0008)))
#define	UMCON2				(*((volatile u32*)(uart2_base + 0x000C)))
#define	UTRSTAT2			(*((volatile u32*)(uart2_base + 0x0010)))
#define	UERSTAT2			(*((volatile u32*)(uart2_base + 0x0014)))
#define	UFSTAT2				(*((volatile u32*)(uart2_base + 0x0018)))
#define	UMSTAT2				(*((volatile u32*)(uart2_base + 0x001C)))
#define	UTXH2				(*((volatile u32*)(uart2_base + 0x0020)))
#define	URXH2				(*((volatile u32*)(uart2_base + 0x0024)))
#define	UBRDIV2				(*((volatile u32*)(uart2_base + 0x0028)))
#define	UFRACVAL2			(*((volatile u32*)(uart2_base + 0x002C)))
#define	UINTP2				(*((volatile u32*)(uart2_base + 0x0030)))
#define	UINTSP2				(*((volatile u32*)(uart2_base + 0x0034)))
#define	UINTMN2				(*((volatile u32*)(uart2_base + 0x0038)))

#define	CLOCK_PHYBASE			((void*)0x10030000)
#define	CLK_SRC_PERIL0_PHYADDR	(CLOCK_PHYBASE + 0xC250)
static	address_t				clk_src_peril0 = (address_t)NULL;
#define	CLK_SRC_PERIL0			(*((volatile u32*)(clk_src_peril0)))

//Seriel configs
#define	BAUD_RATE			115200

//Control characters
#define	CTLR_CLEAR					"\033[2J"
#define	CTLR_CLEAR_LEN				(sizeof(CTLR_CLEAR) - 1)
#define	CTLR_CURSOR_RESET			"\033[0;0H"
#define	CTLR_CURSOR_RESET_LEN		(sizeof(CTLR_CURSOR_RESET) - 1)

static volatile	u32	current_cursor_line;
static volatile	u32	current_cursor_row;

static spnlck_t	lock;

static void		init_UART2();
static void		send_ch(u8 ch);
static void		sends(u8* buf, size_t size);


void hal_early_print_init()
{
    //Initialize lock
    core_pm_spnlck_init(&lock);

    //Initialize serial
    init_UART2();

    //Set bachground color
    //hal_early_print_color(FG_BRIGHT_WHITE, BG_BLACK);
    hal_early_print_color(FG_BRIGHT_WHITE, BG_RED);

    //Clear screen
    hal_early_print_cls();

    return;
}

void hal_early_print_cls()
{
    core_pm_spnlck_lock(&lock);
    sends((u8*)CTLR_CLEAR, CTLR_CLEAR_LEN);
    sends((u8*)CTLR_CURSOR_RESET, CTLR_CURSOR_RESET_LEN);
    core_pm_spnlck_unlock(&lock);

    return;
}

void hal_early_print_color(u32 new_fg, u32 new_bg)
{
    char str[9];
    core_pm_spnlck_lock(&lock);

    core_rtl_snprintf(str, sizeof(str), "\033[%u;%um", new_bg, new_fg);
    sends((u8*)str, core_rtl_strlen(str));

    core_pm_spnlck_unlock(&lock);

    return;
}

void hal_early_print_puts(char* str)
{
    core_pm_spnlck_lock(&lock);

    for(char* p = str;
        *p != '\0';
        p++) {
        if(*p == '\n') {
            send_ch('\r');
            send_ch('\n');

        } else {
            send_ch(*p);
        }
    }

    core_pm_spnlck_unlock(&lock);

    return;
}

void init_UART2()
{
    //Map register memories.
    if(uart2_base == (address_t)NULL) {
        uart2_base = (address_t)hal_mmu_add_early_paging_addr(UART2_PHYBASE);
    }

    if(clk_src_peril0 == (address_t)NULL) {
        clk_src_peril0 = (address_t)hal_mmu_add_early_paging_addr(
                             CLK_SRC_PERIL0_PHYADDR);
    }

    /*
        //Select clock, we use SCLK_USBPHY0 here, It is always 48 MHz.
        CLK_SRC_PERIL0 = (CLK_SRC_PERIL0 & ~0x00000F00) | 0x00000300;

        //Set baud-rate
        // UBRDIV2 = 48000000 / (BAUD_RATE * 16);
        UBRDIV2 = 26;

        //UFRACVAL2 = (48000000.0 / (BAUD_RATE * 16) - UBRDIV2) * 16;
        UFRACVAL2 = 1;
    */
    //Set mode
    ULCON2 = 0x00000003;
    UCON2 = 0x00000005;

    return;
}

void send_ch(u8 ch)
{
    while(!(UTRSTAT2 & 0x02));

    UTXH2 = ch;
    return;
}

void sends(u8* buf, size_t size)
{
    u8* p = buf;

    while(size > 0) {
        send_ch(*p);
        size--;
        p++;
    }

    return;
}
