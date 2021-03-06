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

#include "../early_print/early_print.h"

#include "./paging/paging.h"
#include "./phymem/phymem.h"

#define MODULE_NAME	hal_mmu

void hal_mmu_init()
{
    hal_early_print_printf("\nInitializing mmu module...\n");
    PRIVATE(phymem_init)();
    PRIVATE(paging_init)();
    return;
}
