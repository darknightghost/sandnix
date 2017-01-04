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
#include "../../../hal/exception/exception.h"
typedef unsigned long	longword_t;

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
    //Copied from glibc
    /*
    	On 32-bit hardware, choosing longword_t to be a 32-bit unsigned
    	long instead of a 64-bit uintmax_t tends to give better
    	performance.  On 64-bit hardware, unsigned long is generally 64
    	bits already.  Change this typedef to experiment with
    	performance.
    */
    u8* char_ptr;
    const longword_t *longword_t_ptr;
    longword_t repeated_one;
    longword_t repeated_c;

    /*
    	Handle the first few bytes by reading one byte at a time.
    	Do this until CHAR_PTR is aligned on a longword_t boundary.
    */
    for(char_ptr = (u8*)buf;
        size > 0 && (size_t) char_ptr % sizeof(longword_t) != 0;
        --size, ++char_ptr) {
        if(*char_ptr == ch) {
            return (void *) char_ptr;
        }
    }

    longword_t_ptr = (const longword_t *) char_ptr;

    /*
    	All these elucidatory comments refer to 4-byte longword_ts,
    	but the theory applies equally well to any size longword_ts.
    */

    /*
    	Compute auxiliary longword_t values:
    	repeated_one is a value which has a 1 in every byte.
    	repeated_c has ch in every byte.
    */
    repeated_one = 0x01010101;
    repeated_c = ch | (ch << 8);
    repeated_c |= repeated_c << 16;

    if(0xffffffffU < (longword_t) - 1) {
        repeated_one |= repeated_one << 31 << 1;
        repeated_c |= repeated_c << 31 << 1;

        if(8 < sizeof(longword_t)) {
            size_t i;

            for(i = 64; i < sizeof(longword_t) * 8; i *= 2) {
                repeated_one |= repeated_one << i;
                repeated_c |= repeated_c << i;
            }
        }
    }

    /*
    	Instead of the traditional loop which tests each byte, we will test a
    	longword_t at a time.  The tricky part is testing if *any of the four*
    	bytes in the longword_t in question are equal to ch.  We first use an xor
    	with repeated_c.  This reduces the task to testing whether *any of the
    	four* bytes in longword_t1 is zero.

    	We compute tmp =
    		((longword_t1 - repeated_one) & ~longword_t1) & (repeated_one << 7).
    	That is, we perform the following operations:
    		1. Subtract repeated_one.
    		2. & ~longword_t1.
    		3. & a mask consisting of 0x80 in every byte.
    	Consider what happens in each byte:
    		- If a byte of longword_t1 is zero, step 1 and 2 transform it into 0xff,
    	and step 3 transforms it into 0x80.  A carry can also be propagated
    	to more significant bytes.
    		- If a byte of longword_t1 is nonzero, let its lowest 1 bit be at
    	position k (0 <= k <= 7); so the lowest k bits are 0.  After step 1,
    	the byte ends in a single bit of value 0 and k bits of value 1.
    	After step 2, the result is just k bits of value 1: 2^k - 1.  After
    	step 3, the result is 0.  And no carry is produced.
    	So, if longword_t1 has only non-zero bytes, tmp is zero.
    	Whereas if longword_t1 has a zero byte, call j the position of the least
    	significant zero byte.  Then the result has a zero at positions 0, ...,
    	j-1 and a 0x80 at position j.  We cannot predict the result at the more
    	significant bytes (positions j+1..3), but it does not matter since we
    	already have a non-zero bit at position 8*j+7.

    	So, the test whether any byte in longword_t1 is zero is equivalent to
    	testing whether tmp is nonzero.
    */

    while(size >= sizeof(longword_t)) {
        longword_t longword_t1 = *longword_t_ptr ^ repeated_c;

        if((((longword_t1 - repeated_one) & ~longword_t1)
            & (repeated_one << 7)) != 0) {
            break;
        }

        longword_t_ptr++;
        size -= sizeof(longword_t);
    }

    char_ptr = (u8*) longword_t_ptr;

    /*
    	At this point, we know that either size < sizeof (longword_t), or one of the
    	sizeof (longword_t) bytes starting at char_ptr is == ch.  On little-endian
    	machines, we could determine the first such byte without any further
    	memory accesses, just by looking at the tmp result from the last loop
    	iteration.  But this does not work on big-endian machines.  Choose code
    	that works in both cases.
    */

    for(; size > 0; --size, ++char_ptr) {
        if(*char_ptr == ch) {
            return (void *) char_ptr;
        }
    }

    return NULL;
}

int core_rtl_memcmp(const void* buf1, const void* buf2, size_t size)
{
    longword_t* p_long1;
    longword_t* p_long2;
    u8* p_byte1;
    u8* p_byte2;
    size_t n;
    size_t len_to_cmp;
    u8 ret;

    //Compare the first few bytes to make the pointer aligned
    if(size < sizeof(longword_t) * 2
       || (address_t)buf1 % sizeof(longword_t)
       != (address_t)buf2 % sizeof(longword_t)) {
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
        len_to_cmp = (address_t)buf1 % sizeof(longword_t);
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

    //Compare sizeof(longword_t) bytes each time
    p_long1 = (longword_t*)p_byte1;
    p_long2 = (longword_t*)p_byte2;
    //len_to_cmp = len_to_cmp / sizeof(longword_t) * sizeof(longword_t)
    len_to_cmp = (~((sizeof(longword_t) - 1))) & size;
    //size = size % sizeof(longword_t)
    size = size & (sizeof(longword_t) - 1);

    for(n = 0; n < len_to_cmp;
        n += sizeof(longword_t), p_long1++, p_long2++) {
        if(*p_long1 - *p_long2 != 0) {
            p_byte1 = (u8*)p_long1;
            p_byte2 = (u8*)p_long2;

            for(n = 0;
                n < sizeof(longword_t);
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

    if(((address_t)p_dest & 0x07) == ((address_t)p_src & 0x07)
       && size > 8) {
        //Align the address
        len_to_cp = 8 - ((address_t)p_dest & 0x07);

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

    if(((address_t)p_dest & 0x03) == ((address_t)p_src & 0x03)
       && size > 4) {
        //Align the address
        len_to_cp = 4 - ((address_t)p_dest & 0x03);

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

    if(((address_t)p_dest & 0x01) == ((address_t)p_src & 0x01)
       && size > 2) {
        //Align the address
        len_to_cp = 2 - ((address_t)p_dest & 0x01);

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

    if(size >= 1) {
        hal_rtl_string_movsb(p_dest, p_src, size);
    }

    return dest;
}

void* core_rtl_memmove(void* dest, const void* src, size_t size)
{
    u8* p_src;
    u8* p_dest;
    size_t len_to_cp;
    size_t count;

    if((address_t)dest < (address_t)src) {
        core_rtl_memcpy(dest, src, size);

    } else if((address_t)dest > (address_t)src) {
        p_src = (u8*)src + size;
        p_dest = (u8*)dest + size;

        if(((address_t)p_dest & 0x07) == ((address_t)p_src & 0x07)
           && size > 8) {
            //Align the address
            len_to_cp = (address_t)p_dest & 0x07;

            if(len_to_cp > 0) {
                size -= len_to_cp;
                p_dest -= len_to_cp;
                p_src -= len_to_cp;
                hal_rtl_string_movsb_back(p_dest, p_src, len_to_cp);
            }

            //Copy 8 bytes each time
            if(size >= 8) {
                len_to_cp = (~((size_t)0x07)) & size;
                count = len_to_cp >> 3;
                p_dest -= len_to_cp;
                p_src -= len_to_cp;
                hal_rtl_string_movsq_back(p_dest, p_src, count);
                size -= len_to_cp;
            }

        }

        if(((address_t)p_dest & 0x03) == ((address_t)p_src & 0x03)
           && size > 4) {
            //Align the address
            len_to_cp = (address_t)p_dest & 0x03;

            if(len_to_cp > 0) {
                size -= len_to_cp;
                p_dest -= len_to_cp;
                p_src -= len_to_cp;
                hal_rtl_string_movsb_back(p_dest, p_src, len_to_cp);
            }

            //Copy 8 bytes each time
            if(size >= 4) {
                len_to_cp = (~((size_t)0x03)) & size;
                count = len_to_cp >> 2;
                p_dest -= len_to_cp;
                p_src -= len_to_cp;
                hal_rtl_string_movsl_back(p_dest, p_src, count);
                size -= len_to_cp;
            }
        }

        if(((address_t)p_dest & 0x01) == ((address_t)p_src & 0x01)
           && size > 2) {
            //Align the address
            len_to_cp = (address_t)p_dest & 0x01;

            if(len_to_cp > 0) {
                size -= len_to_cp;
                p_dest -= len_to_cp;
                p_src -= len_to_cp;
                hal_rtl_string_movsb_back(p_dest, p_src, len_to_cp);
            }

            //Copy 8 bytes each time
            if(size >= 2) {
                len_to_cp = (~((size_t)0x01)) & size;
                count = len_to_cp >> 1;
                p_dest -= len_to_cp;
                p_src -= len_to_cp;
                hal_rtl_string_movsw_back(p_dest, p_src, count);
                size -= len_to_cp;
            }
        }

        p_dest -= size;
        p_src -= size;

        if(size >= 1) {
            hal_rtl_string_movsb_back(p_dest, p_src, size);
        }
    }

    return dest;
}

void* core_rtl_memset(void* dest, u8 value, size_t size)
{
    u8* p_dest;
    size_t len_to_set;
    size_t count;

    p_dest = (u8*)dest;

    //Set 8 bytes each time
    if(((address_t)p_dest & 0x07) != 0) {
        len_to_set = 8 - ((address_t)dest & 0x07);
        hal_rtl_string_setsb(p_dest, value, len_to_set);
        p_dest += len_to_set;
        size -= len_to_set;
    }

    if(size >= 8) {
        len_to_set = (~((size_t)0x07)) & size;
        count = len_to_set >> 3;
        size -= len_to_set;
        hal_rtl_string_setsq(p_dest, value, count);
        p_dest += len_to_set;
    }

    //Set 4 bytes each time
    if(((address_t)p_dest & 0x03) != 0) {
        len_to_set = 4 - ((address_t)dest & 0x03);
        hal_rtl_string_setsb(p_dest, value, len_to_set);
        p_dest += len_to_set;
        size -= len_to_set;
    }

    if(size >= 4) {
        len_to_set = (~((size_t)0x03)) & size;
        count = len_to_set >> 2;
        size -= len_to_set;
        hal_rtl_string_setsl(p_dest, value, count);
        p_dest += len_to_set;
    }

    //Set 2 bytes each time
    if(((address_t)p_dest & 0x01) != 0) {
        len_to_set = 2 - ((address_t)dest & 0x01);
        hal_rtl_string_setsb(p_dest, value, len_to_set);
        p_dest += len_to_set;
        size -= len_to_set;
    }

    if(size >= 2) {
        len_to_set = (~((size_t)0x01)) & size;
        count = len_to_set >> 1;
        size -= len_to_set;
        hal_rtl_string_setsw(p_dest, value, count);
        p_dest += len_to_set;
    }

    //Set 1 bytes each time
    if(size >= 1) {
        hal_rtl_string_setsb(p_dest, value, size);
    }

    return dest;
}

char* core_rtl_strchr(const char* str, char c)
{
    //Copied from glibc
    const unsigned char *char_ptr;
    const longword_t *longword_ptr;
    longword_t longword, magic_bits, charmask;
    unsigned char chr;

    chr = (unsigned char) c;

    /*
    	Handle the first few characters by reading one character at a time.
        Do this until CHAR_PTR is aligned on a longword boundary.
    */
    for(char_ptr = (const unsigned char *) str;
        ((unsigned long int) char_ptr & (sizeof(longword) - 1)) != 0;
        ++char_ptr) {
        if(*char_ptr == chr) {
            return (void *) char_ptr;

        } else if(*char_ptr == '\0') {
            return NULL;
        }
    }

    /*
    	All these elucidatory comments refer to 4-byte longwords,
        but the theory applies equally well to 8-byte longwords.
    */

    longword_ptr = (unsigned long int *) char_ptr;

    /*
    	Bits 31, 24, 16, and 8 of this number are zero.  Call these bits
        the "holes."  Note that there is a hole just to the left of
        each byte, with an extra at the end:

        bits:  01111110 11111110 11111110 11111111
        bytes: AAAAAAAA BBBBBBBB CCCCCCCC DDDDDDDD

        The 1-bits make sure that carries propagate to the next 0-bit.
        The 0-bits provide holes for carries to fall into.
    */
    switch(sizeof(longword)) {
        case 4:
            magic_bits = 0x7efefeffL;
            break;

        case 8:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshift-overflow"
            magic_bits = (((0x7efefefeL << 16)) << 16) | 0xfefefeffL;
#pragma GCC diagnostic pop
            break;

        default:
            PANIC(ENOTSUP, "Unsupported Architecture.");

    }

    /*
    	Set up a longword, each of whose bytes is C.
    */
    charmask = chr | (chr << 8);
    charmask |= charmask << 16;

    if(sizeof(longword) > 4) {
        /*
        	Do the shift in two steps to avoid a warning if long has 32 bits.
        */
        charmask |= (charmask << 16) << 16;
    }

    if(sizeof(longword) > 8) {
        PANIC(ENOTSUP, "Unsupported architecture.");
    }

    /*
    	Instead of the traditional loop which tests each character,
        we will test a longword at a time.  The tricky part is testing
        if *any of the four* bytes in the longword in question are zero.
    */
    for(;;) {
        /*
        	We tentatively exit the loop if adding MAGIC_BITS to
        	LONGWORD fails to change any of the hole bits of LONGWORD.

        	1) Is this safe?  Will it catch all the zero bytes?
            Suppose there is a byte with all zeros.  Any carry bits
        	propagating from its left will fall into the hole at its
        	least significant bit and stop.  Since there will be no
        	carry from its most significant bit, the LSB of the
        	byte to the left will be unchanged, and the zero will be
        	detected.

        	2) Is this worthwhile?  Will it ignore everything except
        	zero bytes?  Suppose every byte of LONGWORD has a bit set
        	somewhere.  There will be a carry into bit 8.  If bit 8
        	is set, this will carry into bit 16.  If bit 8 is clear,
        	one of bits 9-15 must be set, so there will be a carry
        	into bit 16.  Similarly, there will be a carry into bit
        	24.  If one of bits 24-30 is set, there will be a carry
        	into bit 31, so all of the hole bits will be changed.

        	The one misfire occurs when bits 24-30 are clear and bit
        	31 is set; in this case, the hole at bit 31 is not
        	changed.  If we had access to the processor carry flag,
        	we could close this loophole by putting the fourth hole
        	at bit 32!

        	So it ignores everything except 128's, when they're aligned
        	properly.

        	3) But wait!  Aren't we looking for C as well as zero?
        	Good point.  So what we do is XOR LONGWORD with a longword,
        	each of whose bytes is C.  This turns each byte that is C
        	into a zero.
        */

        longword = *longword_ptr++;

        /*
        	Add MAGIC_BITS to LONGWORD.
        */
        if((((longword + magic_bits)

             /*
              	Set those bits that were unchanged by the addition.
             */
             ^ ~longword)

            /*
            	Look at only the hole bits.  If any of the hole bits
                are unchanged, most likely one of the bytes was a
                zero.
            */
            & ~magic_bits) != 0 ||

           /*
           	That caught zeroes.  Now test for C.
           */
           ((((longword ^ charmask) + magic_bits) ^ ~(longword ^ charmask))
            & ~magic_bits) != 0) {
            /*
            	Which of the bytes was C or zero?
            	If none of them were, it was a misfire; continue the search.
            */

            const unsigned char *cp = (const unsigned char *)(longword_ptr - 1);

            if(*cp == chr) {
                return (char *) cp;

            } else if(*cp == '\0') {
                return NULL;
            }

            if(*++cp == chr) {
                return (char *) cp;

            } else if(*cp == '\0') {
                return NULL;
            }

            if(*++cp == chr) {
                return (char *) cp;

            } else if(*cp == '\0') {
                return NULL;
            }

            if(*++cp == chr) {
                return (char *) cp;

            } else if(*cp == '\0') {
                return NULL;
            }

            if(sizeof(longword) > 4) {
                if(*++cp == chr) {
                    return (char *) cp;

                } else if(*cp == '\0') {
                    return NULL;
                }

                if(*++cp == chr) {
                    return (char *) cp;

                } else if(*cp == '\0') {
                    return NULL;
                }

                if(*++cp == chr) {
                    return (char *) cp;

                } else if(*cp == '\0') {
                    return NULL;
                }

                if(*++cp == chr) {
                    return (char *) cp;

                } else if(*cp == '\0') {
                    return NULL;
                }
            }
        }
    }

    return NULL;
}

size_t core_rtl_strcspn(const char* str, const char* reject)
{
    size_t ret;
    char* p;

    ret = 0;
    p = (char*)str;

    while(*p != '\0') {
        if(core_rtl_strchr(reject, *(p++)) == NULL) {
            ret++;

        } else {
            return ret;
        }
    }

    return ret;
}

size_t core_rtl_strlen(const char* str)
{
    //Copied from glibc
    const char *char_ptr;
    const unsigned long int *longword_ptr;
    unsigned long int longword, himagic, lomagic;

    /*
    	Handle the first few characters by reading one character at a time.
    	Do this until CHAR_PTR is aligned on a longword boundary.
    */
    for(char_ptr = str; ((unsigned long int) char_ptr
                         & (sizeof(longword) - 1)) != 0;
        ++char_ptr) {
        if(*char_ptr == '\0') {
            return char_ptr - str;
        }
    }

    /*
    	All these elucidatory comments refer to 4-byte longwords,
    	but the theory applies equally well to 8-byte longwords.
    */

    longword_ptr = (unsigned long int *) char_ptr;

    /*
    	Bits 31, 24, 16, and 8 of this number are zero.  Call these bits
        the "holes."  Note that there is a hole just to the left of
        each byte, with an extra at the end:

        bits:  01111110 11111110 11111110 11111111
        bytes: AAAAAAAA BBBBBBBB CCCCCCCC DDDDDDDD

        The 1-bits make sure that carries propagate to the next 0-bit.
        The 0-bits provide holes for carries to fall into.
    */
    himagic = 0x80808080L;
    lomagic = 0x01010101L;

    if(sizeof(longword) > 4) {
        //64-bit version of the magic.
        //Do the shift in two steps to avoid a warning if long has 32 bits.
        himagic = ((himagic << 16) << 16) | himagic;
        lomagic = ((lomagic << 16) << 16) | lomagic;
    }

    if(sizeof(longword) > 8) {
        PANIC(ENOTSUP, "Unsupported architecture.");
    }

    /*
    	Instead of the traditional loop which tests each character,
        we will test a longword at a time.  The tricky part is testing
        if *any of the four* bytes in the longword in question are zero.
    */
    for(;;) {
        longword = *longword_ptr++;

        if(((longword - lomagic) & ~longword & himagic) != 0) {
            /*
            	Which of the bytes was the zero?  If none of them were, it was
                a misfire; continue the search.
            */

            const char *cp = (const char *)(longword_ptr - 1);

            if(cp[0] == 0) {
                return cp - str;
            }

            if(cp[1] == 0) {
                return cp - str + 1;
            }

            if(cp[2] == 0) {
                return cp - str + 2;
            }

            if(cp[3] == 0) {
                return cp - str + 3;
            }

            if(sizeof(longword) > 4) {
                if(cp[4] == 0)
                    return cp - str + 4;

                if(cp[5] == 0)
                    return cp - str + 5;

                if(cp[6] == 0)
                    return cp - str + 6;

                if(cp[7] == 0)
                    return cp - str + 7;
            }
        }
    }
}

char* core_rtl_strncat(char *dest, const char *src, size_t len)
{
    char* p;
    size_t len_to_cp;

    p = dest + core_rtl_strlen(dest);

    //How many charachers to copy
    len_to_cp = core_rtl_strlen(src);

    if(len_to_cp > len) {
        len_to_cp = len;
    }

    //Copy
    p[len_to_cp] = '\0';
    core_rtl_memcpy(p, src, len_to_cp);

    return dest;
}

int core_rtl_strncmp(const char* s1, const char* s2, size_t len)
{
    unsigned char* p1;
    unsigned char* p2;
    int ret;

    for(p1 = (unsigned char*)s1, p2 = (unsigned char*)s2;
        len > 0;
        p1++, p2++, len--) {
        ret = *p1 - *p2;

        if(ret != 0 || *p1 == '\0') {
            return ret;
        }
    }

    return 0;
}

int core_rtl_strcmp(const char* s1, const char* s2)
{
    return core_rtl_strncmp(s1, s2, 0xFFFFFFFF);
}

char* core_rtl_strncpy(char* dest, const char* src, size_t len)
{
    char* p_src;
    char* p_dest;
    size_t count;

    for(count = 0, p_src = (char*)src, p_dest = dest;
        count < len;
        p_src++, p_dest++, count++) {
        *p_dest = *p_src;

        if(p_src == '\0') {
            return dest;
        }
    }

    p_dest = '\0';
    return dest;
}

char* core_rtl_strpbrk(const char* str1, const char* accept)
{
    char* p_s;
    char* p_a;

    for(p_s = (char*)str1;
        *p_s != '\0';
        p_s++) {
        for(p_a = (char*)accept;
            *p_a != '\0';
            p_a++) {
            if(*p_s == *p_a) {
                return p_s;
            }
        }
    }

    return NULL;
}

char* core_rtl_strrchr(const char* str, char ch)
{
    char* p;
    char* found;

    if(ch == '\0') {
        return core_rtl_strchr(str, ch);
    }

    found = NULL;

    for(p = (char*)str;
        *p != '\0';
        p++) {
        if(*p == ch) {
            found = p;
        }
    }

    return found;
}

size_t core_rtl_strspn(const char* str, const char* accept)
{
    size_t ret;
    char* p_s;
    char* p_a;

    for(ret = 0, p_s = (char*)str, p_a = (char*)accept;
        *p_s != '\0';
        p_s++, p_a++, ret++) {
        if(*p_s != *p_a) {
            break;
        }
    }

    return ret;
}

char* core_rtl_strstr(const char* str1, const char* str2)
{
    char* p1;
    char* p2;
    char* p_tmp;

    for(p1 = (char*)str1;
        *p1 != '\0';
        p1++) {
        p2 = (char*)str2;

        if(*p1 == *p2) {
            p_tmp = p1;

            while(true) {
                if(*p2 == '\0') {
                    return p1;

                } else {
                    if(*p_tmp != *p2) {
                        break;
                    }
                }

                p_tmp++;
                p2++;
            }
        }
    }

    return NULL;
}

char* core_rtl_strsplit(const char *str, const char *delim, char* buf, size_t size)
{
    char* p;
    size_t len_to_cp;

    p = core_rtl_strstr(str, delim);

    if(p == NULL) {
        core_rtl_strncpy(buf, str, size - 1);
        return NULL;
    }

    len_to_cp = p - str;

    if(len_to_cp > size - 1) {
        len_to_cp = size - 1;
    }

    core_rtl_strncpy(buf, str, len_to_cp);

    return p + core_rtl_strlen(delim);
}

char* core_rtl_itoa(char* buf, u64 num)
{
    char* p;
    u64 n;
    char* p1;
    char* p2;
    char t;

    if(num == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return buf;
    }

    for(n = num, p = buf;
        n != 0;
        n = hal_rtl_math_div64(n, 10), p++) {
        *p = hal_rtl_math_mod64(n, 10) + '0';
    }

    for(p1 = buf, p2 = p - 1;
        p2 > p1;
        p1++, p2--) {
        t = *p1;
        *p1 = *p2;
        *p2 = t;

    }

    *p = '\0';
    return buf;
}

char* core_rtl_htoa(char* buf, u64 num, bool capital_flag)
{
    char* p;
    u64 n;
    u8 t;
    char* p1;
    char* p2;

    if(num == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return buf;
    }

    for(n = num, p = buf;
        n != 0;
        n = n / 0x10, p++) {
        t = n % 0x10;

        if(t < 0x0A) {
            *p = t + '0';

        } else {
            if(capital_flag) {
                *p = t - 0x0A + 'A';

            } else {
                *p = t - 0x0A + 'a';
            }
        }
    }

    for(p1 = buf, p2 = p - 1;
        p2 > p1;
        p1++, p2--) {
        t = *p1;
        *p1 = *p2;
        *p2 = t;

    }

    *p = '\0';
    return buf;
}

char* core_rtl_otoa(char* buf, u64 num)
{
    char* p;
    char* p1;
    char* p2;
    char t;
    u64 n;

    if(num == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return buf;
    }

    for(n = num, p = buf;
        n != 0;
        n = n / 010, p++) {
        *p = n % 010 + '0';
    }

    for(p1 = buf, p2 = p - 1;
        p2 > p1;
        p1++, p2--) {
        t = *p1;
        *p1 = *p2;
        *p2 = t;

    }

    *p = '\0';
    return buf;
}

s32 core_rtl_atoi(char* str, int num_sys)
{
    char* p;
    u32 len;
    u32 ret;
    len = core_rtl_strlen(str);

    //Check arguments
    if(num_sys > 36) {
        return 0;
    }

    //Convert string
    ret = 0;

    for(p = str; p < str + len; p++) {
        u32 num;

        if(*p >= '0' && *p <= '9' && *p <= '0' + num_sys) {
            num = *p - '0';

        } else if(num_sys >= 16) {
            if(*p >= 'a' && *p <= 'a' + num_sys - 0x0A) {
                num = *p - 'a' + 0x0A;

            } else if(*p >= 'A' && *p <= 'A' + num_sys - 0x0A) {
                num = *p - 'A' + 0x0A;

            } else {
                return ret;
            }

        } else {
            return ret;
        }

        ret = ret * num_sys + num;
    }

    return ret;
}
