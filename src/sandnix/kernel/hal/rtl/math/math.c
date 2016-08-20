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

#include "math.h"

#if defined(X86) || defined(ARM)
u64 hal_rtl_math_div64(u64 dividend, u64 divisor)
{
    u64 quotient;
    int i;
    u64 remainder;

    quotient = 0;
    remainder = 0;

    for(i = 0; i < 64; i++) {
        if(dividend & 0x8000000000000000) {
            remainder = (remainder << 1) + 1;

        } else {
            remainder = remainder << 1;
        }

        if(remainder >= divisor) {
            remainder -= divisor;
            quotient = (quotient << 1) + 1;

        } else {
            quotient = quotient << 1;
        }

        dividend = dividend << 1;
    }

    return quotient;
}

u64 hal_rtl_math_mod64(u64 dividend, u64 divisor)
{
    u64 quotient;
    int i;
    u64 remainder;

    quotient = 0;
    remainder = 0;

    for(i = 0; i < 64; i++) {
        if(dividend & 0x8000000000000000) {
            remainder = (remainder << 1) + 1;

        } else {
            remainder = remainder << 1;
        }

        if(remainder >= divisor) {
            remainder -= divisor;
            quotient = (quotient << 1) + 1;

        } else {
            quotient = quotient << 1;
        }

        dividend = dividend << 1;
    }

    return remainder;
}

u32 hal_rtl_math_div32(u32 dividend, u32 divisor)
{
    u32 quotient;
    int i;
    u32 remainder;

    quotient = 0;
    remainder = 0;

    for(i = 0; i < 32; i++) {
        if(dividend & 0x80000000) {
            remainder = (remainder << 1) + 1;

        } else {
            remainder = remainder << 1;
        }

        if(remainder >= divisor) {
            remainder -= divisor;
            quotient = (quotient << 1) + 1;

        } else {
            quotient = quotient << 1;
        }

        dividend = dividend << 1;
    }

    return quotient;
}

u32 hal_rtl_math_mod32(u32 dividend, u32 divisor)
{
    u32 quotient;
    int i;
    u32 remainder;

    quotient = 0;
    remainder = 0;

    for(i = 0; i < 32; i++) {
        if(dividend & 0x80000000) {
            remainder = (remainder << 1) + 1;

        } else {
            remainder = remainder << 1;
        }

        if(remainder >= divisor) {
            remainder -= divisor;
            quotient = (quotient << 1) + 1;

        } else {
            quotient = quotient << 1;
        }

        dividend = dividend << 1;
    }

    return remainder;
}

#endif
