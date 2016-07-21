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

#include "init.h"
#include "../early_print/early_print.h"
#include "../mmu/mmu.h"
#include "../../core/rtl/rtl.h"
#include "../../hal/exception/exception.h"

static void test();

void kinit(void* p_kparams)
{
    hal_early_print_init();
    hal_early_print_puts(VER_STR);
    hal_early_print_puts(" loading...\n");

    //Analyse parameters
    hal_mmu_add_early_paging_addr(p_kparams);

    test();

    while(1);

    UNREFERRED_PARAMETER(p_kparams);
    return;
}

void test()
{
    char a[] = "123456789abcdefghijklmnopqrstuvwxyz";

    core_rtl_memset(a, '5', 10);

    while(1) {
    }
}
