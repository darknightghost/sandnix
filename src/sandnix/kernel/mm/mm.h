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
#include "phymem/phymem.h"
#include "paging/paging.h"
#ifndef	_ASM
	#include "heap/heap.h"
#endif	//!	_ASM

#ifdef	X86
	#include "arch/x86/page_table.h"
	#include "arch/x86/phymem/phymem.h"
	#include "arch/x86/paging/paging.h"
#endif

#ifdef	AMD64
	#include "arch/amd64/page_table.h"
#endif

#ifndef	_ASM
	void	mm_init();
	void	mm_excpt_hndlr_init();
#endif	//!	_ASM
