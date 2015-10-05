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

#include "string.h"
#include "../io/stdout.h"
#include "../fs/fs.h"

void memset(void* addr, u8 value, size_t size)
{
	__asm__ __volatile__(
	    "cld\n\t"
	    "movl		%0,%%edi\n\t"
	    "movl		%1,%%ecx\n\t"
	    "movb		%2,%%al\n\t"
	    "rep		stosb\n\t"
	    ::"m"(addr), "m"(size), "m"(value)
	    :"ax", "cx", "dx");
	return;
}

void memcpy(void* dest, void* src, size_t len)
{
	__asm__ __volatile__(
	    "cld\n\t"
	    "movl		%0,%%edi\n\t"
	    "movl		%1,%%esi\n\t"
	    "movl		%2,%%ecx\n\t"
	    "rep		movsb"
	    ::"m"(dest), "m"(src), "m"(len)
	    :"cx", "si", "di");
	return;
}

char* strcat(char* dest, char* src)
{
	char* p;
	p = dest + strlen(dest);
	strcpy(p, src);
	return dest;
}

char* strcut(char* dest, char* src, char end_str)
{
	char* p_dest;
	char* p_src;

	for(p_dest = dest, p_src = src;
	    *p_src != '\0' && *p_src != end_str;
	    p_dest++, p_src++) {
		*p_dest = *p_src;
	}

	*p_dest = '\0';
	return dest;
}
size_t strlen(char* str)
{
	size_t ret;
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
	    :"m"(str)
	    :"cx", "di");
	return ret;
}

char* strcpy(char* dest, char* src)
{
	size_t len;
	len = strlen(src) + 1;
	__asm__ __volatile__(
	    "cld\n\t"
	    "xorl		%%eax,%%eax\n\t"
	    "movl		%2,%%ecx\n\t"
	    "movl		%0,%%esi\n\t"
	    "movl		%1,%%edi\n\t"
	    "repnz		movsb\n\t"
	    ::"m"(src), "m"(dest), "m"(len)
	    :"ax", "cx", "si", "di");
	return dest;
}
int strcmp(char* dest, char* src)
{
	int ret;
	char* p1;
	char* p2;

	for(p1 = dest, p2 = src;
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
char* dectostr(u32 num, char* buf)
{
	u32 n;
	char* p1;
	char* p2;
	char t;
	p1 = buf;
	n = num;

	while(1) {
		*p1 = '0' + n % 10;
		p1++;
		n = n / 10;

		if(n == 0) {
			break;
		}
	}

	*p1 = '\0';

	for(p2 = p1 - 1, p1 = buf;
	    p2 > p1;
	    p1++, p2--) {
		t = *p1;
		*p1 = *p2;
		*p2 = t;
	}

	return buf;
}
char* hextostr(u32 num, char* buf)
{
	u32 n;
	char* p1;
	char* p2;
	char t;
	p1 = buf;
	n = num;

	while(1) {
		if(n % 0x10 < 10) {
			*p1 = '0' + n % 0x10;

		} else {
			*p1 = 'A' + n % 0x10 - 0x0A;
		}

		p1++;
		n = n / 0x10;

		if(n == 0) {
			break;
		}
	}

	*p1 = '\0';

	for(p2 = p1 - 1, p1 = buf;
	    p2 > p1;
	    p1++, p2--) {
		t = *p1;
		*p1 = *p2;
		*p2 = t;
	}

	return buf;
}
