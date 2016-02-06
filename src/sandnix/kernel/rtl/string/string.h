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

#include "../../../../common/common.h"
#include "../rtl.h"

void*		rtl_memcpy(void* dest, void* src, size_t len);
void*		rtl_memset(void* dest, u8 val, size_t len);
void*		rtl_memmove(void* dest, void* src, size_t len);
char*		rtl_strncpy(char* dest, size_t buf_size, char* src);
size_t		rtl_strlen(char* str);
s32			rtl_strcmp(char* str1, char* str2);
char*		rtl_strncat(char* dest, size_t buf_size, char* src);
bool		rtl_strcontain(char* str, char* substr);
s64			rtl_atoi(char* str, int num_sys);
char*		rtl_itoa(char* buf, u64 num, int num_sys, bool capital);
u32			rtl_snprintf(char* buf, size_t buf_size, char* fmt, ...);
u32			rtl_vnprintf(char* buf, size_t buf_size, char* fmt, va_list args);
