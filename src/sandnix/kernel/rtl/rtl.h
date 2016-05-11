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

#ifndef	_ASM
#ifdef	X86

//Variable Arguments
typedef	u8*				va_list;

#define	va_start(ap,v)	((ap) = (va_list)&(v) \
                                + (sizeof(v) > 4 ? sizeof(v) : 4))

#define	VA_SIZE(t)		(sizeof(t) > 4 ? sizeof(t) : 4)

#define	va_arg(ap,t)	((ap) += VA_SIZE(t), \
                         *(t*)((ap) - VA_SIZE(t)))

#define	va_end(ap)		((ap) = (va_list)0)
#endif	//!	X86

<<<<<<< HEAD:src/sandnix/rtl/rtl.h
<<<<<<< HEAD
void*		rtl_memcpy(void* dest, void* src, size_t len);
void*		rtl_memset(void* dest, u8 val, size_t len);
void*		rtl_memmove(void* dest, void* src, size_t len);
char*		rtl_strcpy_s(char* dest, size_t buf_size, char* src);
u32			rtl_strlen(char* str);
s32			rtl_strcmp(char* str1, char* str2);
char*		rtl_strcat_s(char* dest, size_t buf_size, char* src);
u32			rtl_sprintf_s(char* buf, size_t buf_size, char* fmt, ...);
u32			rtl_vprintf_s(char* buf, size_t buf_size, char* fmt, va_list args);
s32			rtl_atoi(char* str, int num_sys);
char*		rtl_itoa(char* buf, u64 num);
char*		rtl_htoa(char* buf, u64 num, bool capital_flag);
char*		rtl_otoa(char* buf, u64 num);
//char*		rtl_ftoa(char* buf, u64 num);
=======
#include "string/string.h"
#include "math/math.h"
=======
>>>>>>> upstream/master:src/sandnix/kernel/rtl/rtl.h
#include "list/list.h"
#include "string/kstring.h"
#include "math/math.h"
#include "bitmap/bitmap.h"
/*
#include "queue/queue.h"
#include "stack/stack.h"
<<<<<<< HEAD
>>>>>>> sandnix_parent/master
=======
#include "array_list/array_list.h"
#include "hash_table/hash_table.h"
<<<<<<< HEAD:src/sandnix/rtl/rtl.h
#include "path/path.h"
>>>>>>> sandnix_2015_9_11/master
=======
#include "path/path.h"*/
>>>>>>> upstream/master:src/sandnix/kernel/rtl/rtl.h

#endif	//!	_ASM
