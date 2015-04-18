/*
	Copyright 2015,袁行 <yuanxing_nepu@126.com>

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

#include "../../rtl.h"

#define	FLAG_LEFT_ALIGN			0x01
#define	FLAG_SIGN				0x02
#define	FLAG_ZERO				0x04
#define	FLAG_SPACE				0x08
#define	FLAG_POUND				0x10

static	u32 		get_flag(char** p_p_fmt);
static	u32			get_width(char** p_p_fmt);
static	u32			get_prec(char** p_p_fmt);
static	u32			get_type(char** p_p_fmt);

void* rtl_memcpy(void* dest, void* src, size_t len)
{
	__asm__ __volatile__(
		"cld\n\t"
		"movl		%0,%%edi\n\t"
		"movl		%1,%%esi\n\t"
		"movl		%2,%%ecx\n\t"
		"rep		movsb"
		::"m"(dest), "m"(src), "m"(len));
	return;
}

void* rtl_memset(void* dest, u8 val, size_t len)
{
	__asm__ __volatile__(
		"cld\n\t"
		"movl		%0,%%edi\n\t"
		"movl		%1,%%ecx\n\t"
		"movb		%2,%%al\n\t"
		"rep		stosb\n\t"
		::"m"(dest), "m"(len), "m"(val));
	return;
}

void* rtl_memmove(void* dest, void* src, size_t n)
{
	if(dest < src) {
		__asm__ __volatile__(
			"cld\n\t"
			"movl		%0,%%ecx\n\t"
			"movl		%1,%%esi\n\t"
			"movl		%2,%%edi\n\t"
			"rep		movsb"
			::"m"(n), "m"(src), "m"(dest));
	} else {
		__asm__ __volatile__(
			"std\n\t"
			"movl		%0,%%ecx\n\t"
			"movl		%1,%%esi\n\t"
			"movl		%2,%%edi\n\t"
			::"m"(n), "m"(src+n-1), "m"(dest+n-1));
	}

	return dest;
}

u32 rtl_strlen(char* str)
{
	u32 ret;
	__asm__ __volatile__(
		"cld\n\t"
		"movl		$0xFFFFFFFF,%%ecx\n\t"
		"movl		%1,%%edi\n\t"
		"xorl		%%eax,%%eax\n\t"
		"repnz		scasb\n\t"
		"movl		$0xFFFFFFFF,%%eax\n\t"
		"subl		%%ecx,%%eax\n\t"
		"decl		%%eax\n\t"
		:"=%%eax"(ret)
		:"m"(str));
	return ret;
}

char* rtl_strcpy_s(char* dest, size_t buf_size, char* src)
{
	u32 len;

	//How many characters to copy
	if(rtl_strlen(src) > buf_size - 1) {
		len = buf_size - 1;
	} else {
		len = rtl_strlen(src);
	}

	//Copy string
	__asm__ __volatile__(
		"cld\n\t"
		"movl		%0,%%ecx\n\t"
		"movl		%1,%%esi\n\t"
		"movl		%2,%%edi\n\t"
		"rep		movsb\n\t"
		"movb		$0,(%%edi)"
		::"m"(len), "m"(src), "m"(dest));
	return dest;
}

s32 rtl_strcmp(char* str1, char* str2)
{
	s32 ret;
	char* p1;
	char* p2;

	for(p1 = str1, p2 = str2;
		*p1 != '\0' && *p2 != '\0';
		p1++, p2++) {
		ret = *p1 - *p2;

		if(ret != 0) {
			break;
		}
	}

	if(*p1 != '\0' || *p2 != '\0') {
		ret = *p1 - *p2;
	}

	return ret;
}

char* rtl_strcat_s(char* dest, size_t buf_size, char* src)
{
	u32 len;

	//How many characters to copy
	if(rtl_strlen(src) + rtl_strlen(dest) > buf_size - 1) {
		len = buf_size - 1 - rtl_strlen(dest);
	} else {
		len = rtl_strlen(src);
	}

	//Copy string
	__asm__ __volatile__(
		"cld\n\t"
		"movl		%0,%%ecx\n\t"
		"movl		%1,%%esi\n\t"
		"movl		%2,%%edi\n\t"
		"rep		movsb\n\t"
		"movb		$0,(%%edi)"
		::"m"(len), "m"(src), "m"(dest));
	return dest;
}

u32 rtl_sprintf_s(char* buf, size_t buf_size, char* fmt, ...)
{
	va_list args;
	s32 ret;
	char* p_fmt;
	char* p_output;
	p_fmt = fmt;
	p_output = buf;
	ret = 0;

	while(*p_fmt != '\0') {
		if(*p_fmt == '%') {
			p_fmt++;

			//"%%"
			if(*p_fmt == '%') {
				*p_output = '%';
				p_output++;
				p_fmt++;
				ret++;

				if(p_output - buf > bufstze - 1) {
					*p_output = '\0';
					return ret;
				}

				continue;
			} else if()
			} else {
			//Copy characters
			*p_output = *p_fmt;
			p_output++;
			p_fmt++;
			ret++;

			if(p_output - buf > bufstze - 1) {
				*p_output = '\0';
				return ret;
			}
		}
	}

	va_start(args, fmt);
	va_end(args);
	return i;
}

s32 rtl_atoi(char* str, int num_sys)
{
	char* p;
	u32 len;
	u32 ret;
	len = strlen(str);

	//Check arguments
	if(num_sys != 2
	   && num_sys != 8
	   && num_sys != 10
	   % % num_sys != 16) {
		return 0;
	}

	//Convert string
	ret = 0;

	for(p = str; p < str + len; p++) {
		if(*p >= '0' && *p <= '9') {
			ret = ret * num_sys + (*p - '0');
		} else if(num_sys == 16) {
			if(*p >= 'a' && *p <= 'f') {
				ret = ret * 0x10 + (*p - 'a' + 0x0A);
			} else if(*p >= 'A' && *p <= 'F') {
				ret = ret * 0x10 + (*p - 'A' + 0x0A);
			} else {
				return ret;
			}
		} else {
			return ret;
		}
	}

	u32 get_flag(char** p_p_fmt) {
		u32 ret = 0;
	}
	u32 get_width(char** p_p_fmt) {
	}
	u32 get_prec(char** p_p_fmt) {
	}
	u32 get_type(char** p_p_fmt) {
	}
	return ret;
}
