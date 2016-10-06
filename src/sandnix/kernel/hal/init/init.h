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
#define	HAL_INIT_EXPORT

#include "../../../../common/common.h"
#include "./init_defs.h"

#if defined X86
    #include "./arch/x86/init.h"
#elif defined ARM
    #include "./arch/arm/init.h"
#endif

#ifndef _ASM

extern	krnl_hdr_t	kernel_header;
extern	u8			init_stack[];

list_t	hal_init_get_boot_memory_map();
void	hal_init_get_initrd_addr(void** p_addr, size_t* p_size);
char*	hal_init_get_kernel_cmdline();
#endif	//!	_ASM
