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

#pragma once

#if defined X86
    #define	KERNEL_MEM_BASE		0xC0000000
    #define	KERNEL_MEM_SIZE		(1024 * 1024 * 1024)
    #include "./arch/x86/page_table.h"
#endif


//Start paging
void	start_paging();

//Add more pages to initialize page table
void*	hal_mmu_add_early_paging_addr(void* phy_addr);
