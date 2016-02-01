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

#include "../../../common/common.h"
#include "../../boot/multiboot2.h"

#define	KERNEL_HEADER_MAGIC		0x444E4153
#define	KERNEL_STACK_SIZE		(4096 * 2)

#ifndef	_ASM

extern	u8		init_stack[KERNEL_STACK_SIZE]

#pragma pack(push)
#pragma pack(1)
typedef	struct	_kernel_header_t {
	le32	magic;
	u32		phy_addr;
	u32		virt_addr;
	u32		text_begin;
	u32		text_end;
	u32		data_begin;
	u32		data_end;
	u32		bss_begin;
	u32		bss_end;
	u32		checksum;
} kernel_header_t, *pkernel_header_t;
#pragma pack(pop)
#endif
