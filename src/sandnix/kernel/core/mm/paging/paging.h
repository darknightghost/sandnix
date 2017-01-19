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

#include "../../../../../common/common.h"

#include "./paging_defs.h"

//Initialize module
void	core_mm_paging_init();

//Initialize cpu core
void	core_mm_paging_cpu_core_init(u32 cpuid);

//Release cpu core
void	core_mm_paging_cpu_core_release(u32 cpuid);

//Switch to page table
void	core_mm_switch_to(u32 index);

//Get current page table index
u32 core_mm_get_current_pg_tbl_index();

//Fork page table
void	core_mm_pg_tbl_fork(u32 src_index, u32 dest_index);

//Release page table
void	core_mm_pg_tbl_release(u32 src_index);
