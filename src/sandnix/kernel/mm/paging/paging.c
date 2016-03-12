/*
	  Copyright 2016,暗夜幽灵 <darknightghost.cn@gmail.com>

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

#include "../../../../common/common.h"
#include "../../rtl/rtl.h"
#include "paging.h"

list_t		kernel_free_blocks;
list_t		kernel_using_blocks;

void paging_init();

void*		mm_page_alloc(void* base, size_t num, u32 options);
void*		mm_page_obj_map(void* base, ppage_obj_t p_obj);
void		mm_page_free(void* base);
ppage_obj_t	mm_get_page_obj(void* base);
