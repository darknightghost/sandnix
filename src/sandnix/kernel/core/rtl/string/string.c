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
/*
	Some functions are copied from glibc.
 */

#include "string.h"
#include "../../../hal/rtl/rtl.h"
typedef unsigned long	longword;

void* core_rtl_memccpy(void* dest, const void* src, u8 ch, size_t size)
{
    size_t n;
    u8* p_src;
    u8* p_dest;

    for(p_src = (u8*)src, p_dest = (u8*)dest, n = 0;
        n < size;
        p_src++, p_dest++, n++) {
        *p_dest = *p_src;

        if(*p_src == ch) {
            return (p_src + 1);
        }
    }

    return NULL;
}

void* core_rtl_memchr(const void* buf, u8 ch, size_t size)
{
    /*
    	On 32-bit hardware, choosing longword to be a 32-bit unsigned
    	long instead of a 64-bit uintmax_t tends to give better
    	performance.  On 64-bit hardware, unsigned long is generally 64
    	bits already.  Change this typedef to experiment with
    	performance.
    */
    u8* char_ptr;
    const longword *longword_ptr;
    longword repeated_one;
    longword repeated_c;

    /*
    	Handle the first few bytes by reading one byte at a time.
    	Do this until CHAR_PTR is aligned on a longword boundary.
    */
    for(char_ptr = (u8*)buf;
        size > 0 && (size_t) char_ptr % sizeof(longword) != 0;
        --size, ++char_ptr)
        if(*char_ptr == ch)
            return (void *) char_ptr;

    longword_ptr = (const longword *) char_ptr;

    /*
    	All these elucidatory comments refer to 4-byte longwords,
    	but the theory applies equally well to any size longwords.
    */

    /*
    	Compute auxiliary longword values:
    	repeated_one is a value which has a 1 in every byte.
    	repeated_c has ch in every byte.
    */
    repeated_one = 0x01010101;
    repeated_c = ch | (ch << 8);
    repeated_c |= repeated_c << 16;

    if(0xffffffffU < (longword) - 1) {
        repeated_one |= repeated_one << 31 << 1;
        repeated_c |= repeated_c << 31 << 1;

        if(8 < sizeof(longword)) {
            size_t i;

            for(i = 64; i < sizeof(longword) * 8; i *= 2) {
                repeated_one |= repeated_one << i;
                repeated_c |= repeated_c << i;
            }
        }
    }

    /*
    	Instead of the traditional loop which tests each byte, we will test a
    	longword at a time.  The tricky part is testing if *any of the four*
    	bytes in the longword in question are equal to ch.  We first use an xor
    	with repeated_c.  This reduces the task to testing whether *any of the
    	four* bytes in longword1 is zero.

    	We compute tmp =
    		((longword1 - repeated_one) & ~longword1) & (repeated_one << 7).
    	That is, we perform the following operations:
    		1. Subtract repeated_one.
    		2. & ~longword1.
    		3. & a mask consisting of 0x80 in every byte.
    	Consider what happens in each byte:
    		- If a byte of longword1 is zero, step 1 and 2 transform it into 0xff,
    	and step 3 transforms it into 0x80.  A carry can also be propagated
    	to more significant bytes.
    		- If a byte of longword1 is nonzero, let its lowest 1 bit be at
    	position k (0 <= k <= 7); so the lowest k bits are 0.  After step 1,
    	the byte ends in a single bit of value 0 and k bits of value 1.
    	After step 2, the result is just k bits of value 1: 2^k - 1.  After
    	step 3, the result is 0.  And no carry is produced.
    	So, if longword1 has only non-zero bytes, tmp is zero.
    	Whereas if longword1 has a zero byte, call j the position of the least
    	significant zero byte.  Then the result has a zero at positions 0, ...,
    	j-1 and a 0x80 at position j.  We cannot predict the result at the more
    	significant bytes (positions j+1..3), but it does not matter since we
    	already have a non-zero bit at position 8*j+7.

    	So, the test whether any byte in longword1 is zero is equivalent to
    	testing whether tmp is nonzero.
    */

    while(size >= sizeof(longword)) {
        longword longword1 = *longword_ptr ^ repeated_c;

        if((((longword1 - repeated_one) & ~longword1)
            & (repeated_one << 7)) != 0)
            break;

        longword_ptr++;
        size -= sizeof(longword);
    }

    char_ptr = (u8*) longword_ptr;

    /*
    	At this point, we know that either size < sizeof (longword), or one of the
    	sizeof (longword) bytes starting at char_ptr is == ch.  On little-endian
    	machines, we could determine the first such byte without any further
    	memory accesses, just by looking at the tmp result from the last loop
    	iteration.  But this does not work on big-endian machines.  Choose code
    	that works in both cases.
    */

    for(; size > 0; --size, ++char_ptr) {
        if(*char_ptr == ch)
            return (void *) char_ptr;
    }

    return NULL;
}

int core_rtl_memcmp(const void* buf1, const void* buf2, size_t size)
{
    longword* p_long1;
    longword* p_long2;
    u8* p_byte1;
    u8* p_byte2;
    size_t n;
    size_t len_to_cmp;
    u8 ret;

    //Compare the first few bytes to make the pointer aligned
    if(size < sizeof(longword) * 2
       || (address_t)buf1 % sizeof(longword)
       != (address_t)buf2 % sizeof(longword)) {
        for(p_byte1 = (u8*)buf1, p_byte2 = (u8*)buf2, n = 0;
            n < size;
            p_byte1++, p_byte2++, n++) {
            ret = *p_byte1 - *p_byte2;

            if(ret != 0) {
                return ret;
            }
        }

        return 0;

    } else {
        len_to_cmp = (address_t)buf1 % sizeof(longword);
        size -= len_to_cmp;

        for(p_byte1 = (u8*)buf1, p_byte2 = (u8*)buf2, n = 0;
            n < len_to_cmp;
            p_byte1++, p_byte2++, n++) {
            ret = *p_byte1 - *p_byte2;

            if(ret != 0) {
                return ret;
            }
        }
    }

    //Compare sizeof(longword) bytes each time
    p_long1 = (longword*)p_byte1;
    p_long2 = (longword*)p_byte2;
    //len_to_cmp = len_to_cmp / sizeof(longword) * sizeof(longword)
    len_to_cmp = (~((sizeof(longword) - 1))) & size;
    //size = size % sizeof(longword)
    size = size & (sizeof(longword) - 1);

    for(n = 0; n < len_to_cmp;
        n += sizeof(longword), p_long1++, p_long2++) {
        if(*p_long1 - *p_long2 != 0) {
            p_byte1 = (u8*)p_long1;
            p_byte2 = (u8*)p_long2;

            for(n = 0;
                n < sizeof(longword);
                n++, p_byte1++, p_byte2++) {
                ret = *p_byte1 - *p_byte2;

                if(ret != 0) {
                    return ret;
                }
            }
        }
    }

    //Compare the last few bytes
    p_byte1 = (u8*)p_long1;
    p_byte2 = (u8*)p_long2;

    for(n = 0;
        n < size;
        n++, p_byte1++, p_byte2++) {
        ret = *p_byte1 - *p_byte2;

        if(ret != 0) {
            return ret;
        }
    }

    return 0;
}

void* core_rtl_memcpy(void* dest, const void* src, size_t size)
{
    u8* p_src;
    u8* p_dest;
    size_t len_to_cp;
    size_t count;

    p_src = (u8*)src;
    p_dest = (u8*)dest;

    if(((address_t)dest & 0x07) == ((address_t)src & 0x07)
       && size > 8) {
        //Align the address
        len_to_cp = (address_t)dest & 0x07;

        if(len_to_cp > 0) {
            size -= len_to_cp;
            hal_rtl_string_movsb(p_dest, p_src, len_to_cp);
            p_dest += len_to_cp;
            p_src += len_to_cp;
        }

        //Copy 8 bytes each time
        if(size >= 8) {
            len_to_cp = (~((size_t)0x07)) & size;
            count = len_to_cp >> 3;
            hal_rtl_string_movsq(p_dest, p_src, count);
            size -= len_to_cp;
            p_dest += len_to_cp;
            p_src += len_to_cp;
        }

    }

    if(((address_t)dest & 0x03) == ((address_t)src & 0x03)
       && size > 4) {
        //Align the address
        len_to_cp = (address_t)dest & 0x03;

        if(len_to_cp > 0) {
            size -= len_to_cp;
            hal_rtl_string_movsb(p_dest, p_src, len_to_cp);
            p_dest += len_to_cp;
            p_src += len_to_cp;
        }

        //Copy 8 bytes each time
        if(size >= 4) {
            len_to_cp = (~((size_t)0x03)) & size;
            count = len_to_cp >> 2;
            hal_rtl_string_movsl(p_dest, p_src, count);
            size -= len_to_cp;
            p_dest += len_to_cp;
            p_src += len_to_cp;
        }
    }

    if(((address_t)dest & 0x01) == ((address_t)src & 0x01)
       && size > 2) {
        //Align the address
        len_to_cp = (address_t)dest & 0x01;

        if(len_to_cp > 0) {
            size -= len_to_cp;
            hal_rtl_string_movsb(p_dest, p_src, len_to_cp);
            p_dest += len_to_cp;
            p_src += len_to_cp;
        }

        //Copy 8 bytes each time
        if(size >= 2) {
            len_to_cp = (~((size_t)0x01)) & size;
            count = len_to_cp >> 1;
            hal_rtl_string_movsw(p_dest, p_src, count);
            size -= len_to_cp;
            p_dest += len_to_cp;
            p_src += len_to_cp;
        }
    }

    hal_rtl_string_movsb(p_dest, p_src, size);
    return dest;
}

void* core_rtl_memmove(void* dest, const void* src, size_t size)
{
    if((address_t)dest >= (address_t)src) {
        core_rtl_memcpy(dest, src, size);
    }

    return dest;
}

void* core_rtl_memset(void* dest, u8 value, size_t size);
char* core_rtl_strchr(const char* str, char c);
size_t core_rtl_strcspn(const char* str, const char* reject);
size_t core_rtl_strlen(const char* str);
char* core_rtl_strncat(char *dest, const char *src, size_t len);
int core_rtl_strncmp(const char* dest, const char* src, size_t len);
char* core_rtl_strncpy(char* dest, const char* src, size_t len);
char* core_rtl_strpbrk(const char* str1, const char* str2);
char* core_rtl_strrchr(const char* str, char ch);
size_t core_rtl_strspn(const char* str, const char* accept);
char* core_rtl_strstr(const char* str1, const char* str2);
char* core_rtl_strsplit(const char *str, const char *delim, char* buf, size_t size);
