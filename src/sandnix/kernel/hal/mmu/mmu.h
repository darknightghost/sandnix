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

#include "../../../../common/common.h"
#include "./paging/paging.h"

#define	SANDNIX_KERNEL_PAGE_SIZE	4096
#define	KERNEL_MAX_SIZE				(16 * 1024 * 1024)

#define	PHYMEM_AVAILABLE	0x00
#define	PHYMEM_USED			0x01
#define	PHYMEM_SYSTEM		0x02
#define	PHYMEM_RESERVED		0x03
#define	PHYMEM_BAD			0x04

typedef	struct	_physical_memory_info {
    void*		begin;
    size_t		size;
    u32			type;
} physical_memory_info_t, *pphysical_memory_info_t;
