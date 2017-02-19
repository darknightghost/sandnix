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

#define MODULE_NAME	core_mm

//Initialize module
void	PRIVATE(paging_init)();

//Initialize cpu core
void	PRIVATE(paging_cpu_core_init)(u32 cpu_index);

//Release cpu core
void	PRIVATE(paging_cpu_core_release)(u32 cpu_index);

//Switch to page table
void	core_mm_switch_to(u32 index);

//Get current page table index
u32 core_mm_get_current_pg_tbl_index();

//Fork page table
void	core_mm_pg_tbl_fork(u32 src_index, u32 dest_index);

//Clear user pages
void	core_mm_pg_tbl_clear(u32 index);

//Release page table
void		core_mm_pg_tbl_release(u32 index);

//Allocate pages
void*		core_mm_pg_alloc(void* base_addr, size_t size, u32 options);

//Free pages
void		core_mm_pg_free(void* base_addr);

//Get page object
ppage_obj_t	core_mm_get_pg_obj(void** p_base_addr, void* addr);

//Map page object
void*		core_mm_map(void* addr, ppage_obj_t p_page_obj);

//Commit pages
void		core_mm_commit(void* addr, u32 options);

//Uncommit pages
void		core_mm_uncommit(void* addr);

//Get page attributes
u32			core_mm_get_pg_attr(void* address);

//Set page attributes
u32			core_mm_set_pg_attr(void* address, u32 attr);

#undef MODULE_NAME
