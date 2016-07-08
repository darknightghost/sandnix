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

#define KERNEL_HEADER_MAGIC		0x444E4153

#if defined X86
    #define	INIT_STACK_SIZE		4096
    #include "./arch/x86/init.h"
#elif defined ARM
    #define	INIT_STACK_SIZE		4096
#endif

#ifndef _ASM

#pragma pack(push)
#pragma pack(1)
typedef struct _krnl_hdr_t {
    address_t	magic;
    void*		kernel_start;
    size_t		kernel_size;
    size_t		header_size;
    address_t	checksum;
} krnl_hdr_t, *pkrnl_hdr_t;
#pragma pack(pop)

extern	krnl_hdr_t	kernel_header;
extern	void*		init_stack;
#endif	//!	_ASM
