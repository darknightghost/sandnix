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
#include "../../../../early_print/early_print.h"
#include "../../../../cpu/cpu.h"

#include "tss.h"

static	tss_t		tss_table[MAX_CPU_NUM];

void tss_init()
{
    hal_early_print_printf("Initializing TSS...\n");
    core_rtl_memset(tss_table, 0, sizeof(tss_table));

    for(u32 i = 0; i < MAX_CPU_NUM; i++) {
        tss_table[i].io_map_base_addr = sizeof(tss_t);
        tss_table[i].ss0 = SELECTOR_K_DATA;

        //Fill descriptor
        TSS_DESC_ADDR_SET(i, &tss_table[i]);
    }

    //Load cpu0 TSS
    tss_table[0].esp0 = (u32)hal_cpu_get_stack_base(init_stack,
                        DEFAULT_STACK_SIZE);
    __asm__ __volatile__(
        "ltr	%0\n"
        ::"r"((u16)SELECTOR_TSS(0)));

    return;
}

void hal_io_set_krnl_stack(address_t addr)
{
    tss_table[hal_cpu_get_cpu_index()].esp0 = addr;
    return;
}
