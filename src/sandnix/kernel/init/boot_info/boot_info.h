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
#include "../../../boot/multiboot2.h"
#include "../../rtl/rtl.h"

typedef	struct	_boot_info {
	void*	initrd_begin;
	size_t	initrd_size;
	list_t	phy_mem_info;
	list_t	kernel_params;
} boot_info_t, *pboot_info_t;

typedef	struct	_kernel_param {
	char*		name;
	char*		value;
} kernel_param_t, *pkernel_param_t;

typedef	struct	_phy_mem_info {
	void*	start_addr;
	size_t	size;
	u32		type;
} phy_mem_info_t, *pphy_mem_info_t;
