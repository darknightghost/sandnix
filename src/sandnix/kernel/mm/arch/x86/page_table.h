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

#include "../../../../../common/common.h"

#define	PG_NP				0
#define	PG_P				1
#define	PG_RDONLY			0
#define	PG_RW				1
#define	PG_SUPERVISOR		0
#define	PG_USER				1
#define	PG_WRITE_BACK		0
#define	PG_WRITE_THROUGH	1
#define	PG_ENCACHE			0
#define	PG_DISACHE			1
#define	PG_SIZE_4K			0

#ifndef	_ASM
#pragma pack(push)
#pragma pack(1)

//Page-Directory Entry
typedef	struct {
	u32		present				: 1;
	u32		read_write			: 1;
	u32		user_supervisor		: 1;
	u32		write_through		: 1;
	u32		cache_disabled		: 1;
	u32		accessed			: 1;
	u32		reserved			: 1;
	u32		page_size			: 1;
	u32		global_page			: 1;
	u32		avail				: 3;
	u32		page_table_base_addr: 20;
} pde_t, *ppde_t;

//Page-Table Entry
typedef	struct {
	u32		present					: 1;
	u32		read_write				: 1;
	u32		user_supervisor			: 1;
	u32		write_through			: 1;
	u32		cache_disabled			: 1;
	u32		accessed				: 1;
	u32		dirty					: 1;
	u32		page_table_attr_index	: 1;
	u32		global_page				: 1;
	u32		avail					: 3;
	u32		page_base_addr			: 20;
} pte_t, *ppte_t;

//#PF error code
typedef	struct	{
	u32		present			: 1;
	u32		read_write		: 1;
	u32		u_s				: 1;
	u32		rsvd			: 1;
	u32		i_d				: 1;
	u32		reserved		: 17;
} pf_err_code_t, *ppf_err_code_t;

#pragma pack(pop)
#endif	//!	_ASM
