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

#include "../../../../../../../../common/common.h"

/* MMU page table */
typedef	struct	_tlb_entry {
    u32		vitrual_page_number	: 20;
    u32		valid				: 1;
    u32		page_size			: 2;
    u32		phypage_base_addr1	: 9;
    u16		phypage_base_addr2	: 11;
    u16		shareable			: 1;
    u16		none_secure			: 1;
    u16		access_permission	: 3;
} __attribute__((aligned(1))) tlb_entry_t, *ptlb_entry_t;
