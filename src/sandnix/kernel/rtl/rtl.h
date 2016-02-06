/*
	Copyright 2015,暗夜幽灵 <darknightghost.cn@gmail.com>

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

#ifdef	X86

	//Variable Arguments
	typedef	u8*				va_list;

	#define	va_start(ap,v)	((ap) = (va_list)&(v) + 4)

	#define	va_arg(ap,t)	((ap) += 4 ,(**((t**)((ap)-4))))

	#define	va_end(ap)		((ap) = (va_list)0)
#endif	//!	X86

#include "string/string.h"
/*#include "math/math.h"
#include "list/list.h"
#include "queue/queue.h"
#include "stack/stack.h"
#include "array_list/array_list.h"
#include "hash_table/hash_table.h"
#include "path/path.h"*/

