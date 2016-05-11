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
#include "../../rtl/rtl.h"
#include "../../om/om.h"
//#include "../phymem/phymem.h"

typedef	struct	_swap_obj {
	kobject_t	obj;
	pkstring_t	device;
	u64			block;
	u64			num;
} swap_obj_t, *pswap_obj_t;

//pphymem_obj_t	swap_load(pswap_obj_t p_swap_obj);
//pswap_obj_t		swap_mem(pphymem_obj_t p_phy_mem);
