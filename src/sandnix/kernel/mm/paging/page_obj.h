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
#include "../swap/swap.h"

#define	PAGE_UNSWAPPABLE	0
#define	PAGE_AVAILABLE		1
#define	PAGE_SWAPABLE		2

#define	MEMBLOCK_EMPTY		0
#define	MEMBLOCK_PHY		1
#define	MEMBLOCK_SWAP		2

typedef	struct	_memblock {
	u32					type;
	union {
		pphymem_block_t	p_phy_mem;
		pswap_obj_t		p_swap_mem;
	};
} memblock_t, *pmemblock_t;

typedef	struct	_page_obj {
	kobject_t		obj;
	u32				status;
	list_t			mem_list;
} page_obj_t, *ppage_obj_t;


