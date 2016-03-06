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

#pragma once

#include "../../../../common/common.h"
#include "../../om/om.h"
#include "../../rtl/rtl.h"
#include "../phymem/phymem.h"

#define	PAGE_BLOCK_SWAPPED		0
#define	PAGE_BLOCK_AVAILABLE	1

typedef	struct	_page_block {
	u32				status;
	union {
		pphymem_obj_t	p_phy_mem_obj;
		void*			p_swaped_mem_obj;
	}
} page_block_t, *ppage_block_t;

typedef	struct	_page_obj {
	kobject_t		obj;
	list_t			pageblock_list;
} page_obj_t, *ppage_obj_t;
