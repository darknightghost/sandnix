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

#include "string.h"

#ifdef	X86

#define	WORD_LEN	4

#define	BYTE_CP_FWD(p_dest,p_src,len){ \
		__asm__ __volatile__( \
		                      "cld\n" \
		                      "rep	movsb\n" \
		                      :"=S"(p_src),"=D"(p_dest) \
		                      :"0"(p_src),"1"(p_dest),"cx"(len) \
		                      :"memory"); \
	}

#define	BYTE_CP_BWD

#define	WORD_CP_FWD(p_dest,p_src,len){ \
		__asm__ __volatile__( \
		                      "cld\n" \
		                      "rep	movsl\n" \
		                      : "=S"(p_src), "=D"(p_dest) \
		                      :"0"(p_src), "1"(p_dest),"cx"(len / 4) \
		                      :"memory"); \
	}

#define	WORD_CP_BWD

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
		//I don't know why,I learnt it form glibc and it really works.
		len_to_cp = (-len) % WORD_LEN;
		len -= len_to_cp;
		BYTE_CP_FWD(p_dest, p_src, len_to_cp);

		WORD_CP_FWD(p_dest, p_src, len);

		len_to_cp = len % WORD_LEN;
		BYTE_CP_FWD(p_dest, p_src, len_to_cp);
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

		len_to_fill = (-len) % WORD_LEN;
		len -= len_to_fill;
		BYTES_FILL(p_dest, len_to_fill, val);
		WORDS_FILL(p_dest, len, val_word);
		len_to_fill = len % WORD_LEN;
		BYTES_FILL(p_dest, len_to_fill, val);
	}

	return dest;
}

void* rtl_memmove(void* dest, void* src, size_t len);
u32 rtl_strlen(char* str);
char* rtl_strcpy_s(char* dest, size_t buf_size, char* src);
s32 rtl_strcmp(char* str1, char* str2);
char* rtl_strcat_s(char* dest, size_t buf_size, char* src);
bool rtl_is_sub_string(char* str, char* substr);
s32 rtl_atoi(char* str, int num_sys);
char* rtl_itoa(char* buf, u64 num);
char* rtl_htoa(char* buf, u64 num, bool capital_flag);
char* rtl_otoa(char* buf, u64 num);
char* rtl_ftoa(char* buf, u64 num);
u32 rtl_sprintf_s(char* buf, size_t buf_size, char* fmt, ...);
u32 rtl_vprintf_s(char* buf, size_t buf_size, char* fmt, va_list args);
