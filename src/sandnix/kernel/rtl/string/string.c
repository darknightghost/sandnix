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

/*
	  I learnt a lot from glic.The art of codes in glibc is brilliant.
*/

#include "string.h"
#include "../math/math.h"

#ifdef	X86

#define	WORD_LEN	4

#define	GET_CHAR_NUM(c,n){ \
		if((c) >= '0' && (c) <= '9') { \
			(n) = (c) - '0'; \
		} else if((c) >= 'a' && (c) <= 'z') { \
			(n) = (c) - 'a' + 10; \
		} else if((c) >= 'A' && (c) <= 'Z') { \
			(n) = (c) - 'A' + 10; \
		} else { \
			(n) = -1; \
		} \
	}

#define	GET_NUM_CHAR(n,c,capital){ \
		if((n) < 10) { \
			(c) = (n) + '0'; \
		} else { \
			if((capital)) { \
				(c) = (n) + 'A'; \
			} else { \
				(c) = (n) + 'a'; \
			} \
		} \
	}

#define	BYTE_CP_FWD(p_dest,p_src,len){ \
		__asm__ __volatile__( \
		                      "cld\n" \
		                      "rep	movsb\n" \
		                      :"=S"(p_src),"=D"(p_dest) \
		                      :"0"(p_src),"1"(p_dest),"cx"(len) \
		                      :"memory"); \
	}

#define	BYTE_CP_BWD(p_dest,p_src,len){ \
		__asm__ __volatile__( \
		                      "std\n" \
		                      "rep	movsb\n" \
		                      :"=S"(p_src),"=D"(p_dest) \
		                      :"0"(p_src),"1"(p_dest),"cx"(len) \
		                      :"memory"); \
	}

#define	WORD_CP_FWD(p_dest,p_src,len){ \
		__asm__ __volatile__( \
		                      "cld\n" \
		                      "rep	movsl\n" \
		                      : "=S"(p_src), "=D"(p_dest) \
		                      :"0"(p_src), "1"(p_dest),"cx"(len / 4) \
		                      :"memory"); \
	}

#define	WORD_CP_BWD(p_dest,p_src,len){ \
		__asm__ __volatile__( \
		                      "std\n" \
		                      "rep	movsl\n" \
		                      : "=S"(p_src), "=D"(p_dest) \
		                      :"0"(p_src), "1"(p_dest),"cx"(len / 4) \
		                      :"memory"); \
	}

#define	BYTES_FILL(p_dest,len,value){ \
		__asm__ __volatile__( \
		                      "cld\n" \
		                      "rep	stosb\n" \
		                      :"=D"(p_dest) \
		                      :"0"(p_dest),"a"(value),"c"(len) \
		                      :"memory"); \
	}

#define	WORDS_FILL(p_dest,len,value){ \
		__asm__ __volatile__( \
		                      "cld\n" \
		                      "rep	stosl\n" \
		                      :"=D"(p_dest) \
		                      :"0"(p_dest),"a"(value),"c"(len / 4) \
		                      :"memory"); \
	}

#define STRLEN_MAGIC	0x7EFEFEFFL
#endif	//X86

void* rtl_memcpy(void* dest, void* src, size_t len)
{
	size_t len_to_cp;
	void* p_src;
	void* p_dest;

	p_src = src;
	p_dest = dest;

	if(len < WORD_LEN * 2) {
		BYTE_CP_FWD(p_dest, p_src, len);

	} else {
		//I don't know why, I learnt it form glibc and it really works.
		len_to_cp = (-(size_t)p_dest) % WORD_LEN;
		len -= len_to_cp;
		BYTE_CP_FWD(p_dest, p_src, len_to_cp);

		WORD_CP_FWD(p_dest, p_src, len);

		len_to_cp = len % WORD_LEN;

		if(len_to_cp > 0) {
			BYTE_CP_FWD(p_dest, p_src, len_to_cp);
		}
	}

	return dest;
}

void* rtl_memset(void* dest, u8 val, size_t len)
{
	size_t val_word;
	void* p_dest;
	size_t len_to_fill;
	int i;

	p_dest = dest;

	if(len < 2 * WORD_LEN) {
		BYTES_FILL(p_dest, len, val);

	} else {
		val_word = 0;

		for(i = 0; i < WORD_LEN; i++) {
			val_word = (val_word << 8) | val;
		}

		len_to_fill = (-(size_t)p_dest) % WORD_LEN;
		len -= len_to_fill;
		BYTES_FILL(p_dest, len_to_fill, val);
		WORDS_FILL(p_dest, len, val_word);
		len_to_fill = len % WORD_LEN;

		if(len_to_fill > 0) {
			BYTES_FILL(p_dest, len_to_fill, val);
		}
	}

	return dest;
}

void* rtl_memmove(void* dest, void* src, size_t len)
{
	u8* p_src;
	u8* p_dest;
	size_t len_to_cp;

	if((size_t)dest + len <= (size_t)src || (size_t)src < (size_t)dest) {
		//Memory not overlapped or src < dest
		rtl_memcpy(dest, src, len);

	} else {
		//Memory overlapped and src > dest
		p_dest = dest + len - 1;
		p_src = src + len - 1;

		if(len < WORD_LEN * 2) {
			BYTE_CP_BWD(p_dest, p_src, len);

		} else {
			len_to_cp = (size_t)dest % WORD_LEN;

			if(len_to_cp > 0) {
				BYTE_CP_BWD(p_dest, p_src, len_to_cp);
			}

			p_dest -= WORD_LEN - 1;
			p_src -= WORD_LEN - 1;
			len -= len_to_cp;
			WORD_CP_BWD(p_dest, p_src, len);

			len_to_cp = len % WORD_LEN;

			if(len_to_cp > 0) {
				BYTE_CP_BWD(p_dest, p_src, len_to_cp);
			}
		}
	}

	return dest;
}

size_t rtl_strlen(char* str)
{
	u8* p;
	size_t* p_long_word;
	size_t len_to_align;

	len_to_align = (-(size_t)str) % WORD_LEN;

	//Align the pointer with 8 bytes
	for(p = (u8*)str;
	    p - (u8*)str < len_to_align;
	    p++) {
		if(*p == '\0') {
			return p - (u8*)str;
		}
	}

	//These codes are learnt form glibc.Test a word each time
	p_long_word = (size_t*)str;

	while(1) {
		if((((*p_long_word + STRLEN_MAGIC) ^ ~*p_long_word)
		    & ~STRLEN_MAGIC) != 0) {
			const u8 *cp = (u8*)(*p_long_word - 1);

			for(int i = 0; i < WORD_LEN; i++) {
				if(cp[i] == '\0') {
					return cp - (u8*)str + i;
				}
			}
		}

		p_long_word++;
	}
}

char* rtl_strncpy(char* dest, size_t buf_size, char* src)
{
	char* p_src;
	char* p_dest;

	p_src = src;
	p_dest = dest;

	while(1) {
		if(p_src - src >= buf_size) {
			*p_dest = '\0';
			break;

		} else if(*p_src == '\0') {
			*p_dest = '\0';
			break;

		} else {
			*p_dest = *p_src;
		}

		p_dest++;
		p_src++;
	}

	return dest;
}

s32 rtl_strcmp(char* str1, char* str2)
{
	char* p1;
	char* p2;

	for(p1 = str1, p2 = str2;
	    *p1 == *p2;
	    p1++, p2++) {
		if(*p1 == '\0') {
			break;
		}
	}

	return *p1 - *p2;
}

char* rtl_strncat(char* dest, size_t buf_size, char* src)
{
	size_t len;
	size_t dest_len;

	dest_len = rtl_strlen(dest);

	len = buf_size - dest_len;
	rtl_strncpy(dest + dest_len, len, src);

	return dest;
}

bool rtl_strcontain(char* str, char* substr)
{
	char* p_str1;
	char* p_str2;
	char* p_substr;

	if(*substr == '\0') {
		return true;
	}

	for(p_str1 = str; *p_str1 != '\0'; p_str1++) {
		if(*p_str1 == *substr) {
			//If the first character matched
			for(p_str2 = p_str1, p_substr = substr;
			    true;
			    p_str2++, p_substr++) {
				//Match other characters
				if(*p_substr == '\0') {
					//Matched
					return true;

				} else if(*p_str2 != *p_substr) {
					break;
				}
			}
		}
	}

	return false;
}

s64 rtl_atoi(char* str, int num_sys)
{
	int sign;
	char* p;
	s64 ret;
	int tmp;

	p = str;

	if(*p == '-') {
		sign = -1;
		p++;

	} else {
		sign = 1;
	}

	ret = 0;

	for(; p != '\0'; p++) {
		GET_CHAR_NUM(*p, tmp);

		if(tmp >= num_sys || tmp < 0) {
			return ret * sign;
		}

		ret = ret * num_sys + tmp;
	}

	return ret * sign;
}

char* rtl_itoa(char* buf, u64 num, int num_sys, bool capital)
{
	char* p1;
	char* p2;
	char* start;

	//Sign
	if(num < 0) {
		*buf = '-';
		start = buf + 1;
		num = -num;

	} else {
		start = buf;
	}

	//Num
	for(p1 = start; num != 0; p1++) {
		int t = num - rtl_div64(num, num_sys) * num_sys;
		num = rtl_div64(num, num_sys);
		GET_NUM_CHAR(t, *p1, capital);
	}

	*p1 = '\0';

	//Reserve
	p2 = p1 - 1;

	for(p1 = start; p2 > p1; p1++, p2--) {
		char t = *p1;
		*p1 = *p2;
		*p2 = t;
	}

	return buf;
}

