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

#include "../../../../../../common/common.h"
#include "../../../../core/rtl/obj/obj_defs.h"
#include "./ipi_arg_obj_defs.h"

typedef	struct	_ipi_arg_tlb_refresh {
    ipi_arg_obj_t	base;
    u32				process_id;
    address_t		virt_addr;
    u32				page_count;
} ipi_arg_tlb_refresh_t, *pipi_arg_tlb_refresh_t;

