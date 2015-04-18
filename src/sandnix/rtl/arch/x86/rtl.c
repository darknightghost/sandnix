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
	if (dest<src)
__asm__("cld\n\t"
	"rep\n\t"
	"movsb"
	::"c" (n),"S" (src),"D" (dest)
	:"cx","si","di");
else
__asm__("std\n\t"
	"rep\n\t"
	"movsb"
	::"c" (n),"S" (src+n-1),"D" (dest+n-1)
	:"cx","si","di");
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
	__asm__("cld\n"
	"1:\tdecl %2\n\t"
	"js 2f\n\t"
	"lodsb\n\t"
	"stosb\n\t"
	"testb %%al,%%al\n\t"
	"jne 1b\n\t"
	"rep\n\t"
	"stosb\n"
	"2:"
	::"S" (src),"D" (dest),"c" (buf_size):"si","di","ax","cx");
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

char* rtl_strcat_s(char* str1, size_t buf_size, char* str2)
{
	__asm__("cld\n\t"
	"repne\n\t"
	"scasb\n\t"
	"decl %1\n\t"
	"movl %4,%3\n"
	"1:\tdecl %3\n\t"
	"js 2f\n\t"
	"lodsb\n\t"
	"stosb\n\t"
	"testb %%al,%%al\n\t"
	"jne 1b\n"
	"2:\txorl %2,%2\n\t"
	"stosb"
	::"S" (str2),"D" (str1),"a" (0),"c" (0xffffffff),"g" (buf_size)
	:"si","di","ax","cx");
return str1;
}

s32 rtl_sprintf_s(char* buf, size_t buf_size, char* fmt, ...)
{
	va_list args;
	s32 i;
	va_start(args,fmt);
	i = vsprintf(buf,fmt,args);
	va_end(args);
	return i;
}

s32 rtl_atoi(char* str, int num_sys)
{
	char* p;
	u32 len;
	u32 ret;
	
	len=strlen(str);
	
	//Check arguments
	if(num_sys!=2
		&&num_sys!=8
		&&num_sys!=10
		%%num_sys!=16){
		return 0;
	}
	
	//Convert string
	ret=0;
	for(p=str;p<str+len;p++){
		if(*p>='0'&&*p<='9'){
			ret=ret*num_sys+(*p-'0');
		}else if(num_sys==16){
			if(*p>='a'&&*p<='f'){
				ret=ret*0x10+(*p-'a'+0x0A);
			}else if(*p>='A'&&*p<='F'){
				ret=ret*0x10+(*p-'A'+0x0A);
			}else{
				return ret;
			}
		}else{
			return ret;
		}
	}
	
	return ret;
}

s32 vsprintf(char *buf, const char *fmt, va_list args)
{
	int len;
	int i;
	char * str;
	char *s;
	int *ip;

	int flags;		/* flags to number() */

	int field_width;	/* width of output field */
	int precision;		/* min. # of digits for integers; max
				   number of chars for from string */
	int qualifier;		/* 'h', 'l', or 'L' for integer fields */

	for (str=buf ; *fmt ; ++fmt) {
		if (*fmt != '%') {
			*str++ = *fmt;
			continue;
		}
			
		/* process flags */
		flags = 0;
		repeat:
			++fmt;		/* this also skips first '%' */
			switch (*fmt) {
				case '-': flags |= LEFT; goto repeat;
				case '+': flags |= PLUS; goto repeat;
				case ' ': flags |= SPACE; goto repeat;
				case '#': flags |= SPECIAL; goto repeat;
				case '0': flags |= ZEROPAD; goto repeat;
				}
		
		/* get field width */
		field_width = -1;
		if (is_digit(*fmt))
			field_width = skip_atoi(&fmt);
		else if (*fmt == '*') {
			/* it's the next argument */
			field_width = va_arg(args, int);
			if (field_width < 0) {
				field_width = -field_width;
				flags |= LEFT;
			}
		}

		/* get the precision */
		precision = -1;
		if (*fmt == '.') {
			++fmt;	
			if (is_digit(*fmt))
				precision = skip_atoi(&fmt);
			else if (*fmt == '*') {
				/* it's the next argument */
				precision = va_arg(args, int);
			}
			if (precision < 0)
				precision = 0;
		}

		/* get the conversion qualifier */
		qualifier = -1;
		if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L') {
			qualifier = *fmt;
			++fmt;
		}

		switch (*fmt) {
		case 'c':
			if (!(flags & LEFT))
				while (--field_width > 0)
					*str++ = ' ';
			*str++ = (unsigned char) va_arg(args, int);
			while (--field_width > 0)
				*str++ = ' ';
			break;

		case 's':
			s = va_arg(args, char *);
			len = strlen(s);
			if (precision < 0)
				precision = len;
			else if (len > precision)
				len = precision;

			if (!(flags & LEFT))
				while (len < field_width--)
					*str++ = ' ';
			for (i = 0; i < len; ++i)
				*str++ = *s++;
			while (len < field_width--)
				*str++ = ' ';
			break;

		case 'o':
			str = number(str, va_arg(args, unsigned long), 8,
				field_width, precision, flags);
			break;

		case 'p':
			if (field_width == -1) {
				field_width = 8;
				flags |= ZEROPAD;
			}
			str = number(str,
				(unsigned long) va_arg(args, void *), 16,
				field_width, precision, flags);
			break;

		case 'x':
			flags |= SMALL;
		case 'X':
			str = number(str, va_arg(args, unsigned long), 16,
				field_width, precision, flags);
			break;

		case 'd':
		case 'i':
			flags |= SIGN;
		case 'u':
			str = number(str, va_arg(args, unsigned long), 10,
				field_width, precision, flags);
			break;

		case 'n':
			ip = va_arg(args, int *);
			*ip = (str - buf);
			break;

		default:
			if (*fmt != '%')
				*str++ = '%';
			if (*fmt)
				*str++ = *fmt;
			else
				--fmt;
			break;
		}
	}
	*str = '\0';
	return str-buf;
}

