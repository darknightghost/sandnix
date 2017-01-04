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

#include "../../../../../common/common.h"
#include "../varg.h"

//String functions
void*		core_rtl_memccpy(void* dest, const void* src, u8 ch, size_t size);
void*		core_rtl_memchr(const void* buf, u8 ch, size_t size);
int			core_rtl_memcmp(const void* buf1, const void* buf2, size_t size);
void*		core_rtl_memcpy(void* dest, const void* src, size_t size);
void*		core_rtl_memmove(void* dest, const void* src, size_t size);
void*		core_rtl_memset(void* dest, u8 value, size_t size);

char*		core_rtl_strchr(const char* str, char c);
size_t		core_rtl_strcspn(const char* str, const char* reject);
size_t		core_rtl_strlen(const char* str);
char*		core_rtl_strncat(char *dest, const char *src, size_t len);
int			core_rtl_strncmp(const char* s1, const char* s2, size_t len);
int			core_rtl_strcmp(const char* s1, const char* s2);
char*		core_rtl_strncpy(char* dest, const char* src, size_t len);
char*		core_rtl_strpbrk(const char* str1, const char* accept);
char*		core_rtl_strrchr(const char* str, char ch);
size_t		core_rtl_strspn(const char* str, const char* accept);
char*		core_rtl_strstr(const char* str1, const char* str2);

char*		core_rtl_strsplit(const char *str, const char *delim, char* buf, size_t size);

char*		core_rtl_itoa(char* buf, u64 num);
char*		core_rtl_otoa(char* buf, u64 num);
char*		core_rtl_htoa(char* buf, u64 num, bool capital_flag);

s32			core_rtl_atoi(char* str, int num_sys);

char*		core_rtl_snprintf(char* buf, size_t size, const char* fmt, ...);
char*		core_rtl_vsnprintf(char* buf, size_t size, const char* fmt, va_list ap);
char*		core_rtl_kprintf(const char* fmt, ...);


