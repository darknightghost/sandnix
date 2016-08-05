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

#include "../../../../../early_print.h"
#include "../../../../../../mmu/mmu.h"
#include "../../../../../../../core/pm/pm.h"

#define	DEFAULT_STDOUT_WIDTH	80
#define	DEFAULT_STDOUT_HEIGHT	25

//Serial reisters
#define	UART2_PHYBASE		0x13820000
static	address_t			uart2_base = NULL;
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

#define	CLOCK_PHYBASE		0x10030000
static	address_t			clock_base = NULL;
#define	CLK_SRC_PERIL0		(*((volatile u32*)(clock_base + 0xC250)))

static volatile	u32	current_cursor_line;
static volatile	u32	current_cursor_row;

static volatile	u8	bg;
static volatile	u8	fg;

static spnlck_t	lock;

static void		init_UART2();
static void		send_ch(u8 ch);


void hal_early_print_init()
{
    //Initialize lock
    core_pm_spnlck_init(&lock);

    //Initialize serial
    init_UART2();

    //Set bachground color
    hal_early_print_color(FG_BRIGHT_WHITE, BG_BLACK);

    //Clear screen
    hal_early_print_cls();

    return;
}

void hal_early_print_cls()
{
    core_pm_spnlck_lock(&lock);

    core_pm_spnlck_unlock(&lock);

    return;
}
/*
void hal_early_print_color(u32 new_fg, u32 new_bg)
{
    core_pm_spnlck_lock(&lock);

    fg = new_fg;
    bg = new_bg;

    core_pm_spnlck_unlock(&lock);

    return;
}

void hal_early_print_puts(char* str)
{
    core_pm_spnlck_lock(&lock);

    core_pm_spnlck_raw_unlock(&lock);

    return;
}

*/

void init_UART2()
{
    //Map register memories.
    if(uart2_base == NULL) {
        uart2_base = hal_mmu_add_early_paging_addr(UART2_PHYBASE);
    }

    if(clock_base == NULL) {
        clock_base = hal_mmu_add_early_paging_addr(CLOCK_PHYBASE);
    }

    //Select clock, we use SCLK_USBPHY0 here, It is always 48 MHz.
    CLK_SRC_PERIL0 = (CLK_SRC_PERIL0 & ~0x00000F00) | 0x00000300;

    //Set baud-rate
}

void send_ch(u8 ch);
