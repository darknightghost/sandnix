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


#define ATOMIC_XADDL(dest, src) { \
        do { \
            __asm__ __volatile ( \
                                 "_XADD:\n" \
                                 "ldrex	r1, [%1]\n" \
                                 "movl	r2, r1\n" \
                                 "addl	r1, %0, r1\n" \
                                 "strex	r3, r1, [%1]\n" \
                                 "teq	r3, #0\n" \
                                 "bne	_XADD\n" \
                                 "movl	%0, r2\n" \
                                 :"+r0"((volatile u32)(src)), \
                                 "m"((volatile u32)(dest)) \
                                 ::"memory", "r1", "r2", "r3"); \
        } while(0); \
    }


#define ATOMIC_CMPXMOVL(dest, src, cmp, result) { \
        do { \
            __asm__ __volatile ( \
                                 "movl		%3, #0\n" \
                                 "ldrex 	r3, [%2]\n" \
                                 "teql		%1, r3\n" \
                                 "moveql	r3, %0\n" \
                                 "moveql	%3, #1\n" \
                                 "strex		r4, r3, [%2]\n" \
                                 "teql		r4, #0\n" \
                                 "movnel	%3, #0\n" \
                                 :"=r0"((volatile u32)(src)), \
                                 "=r1"((volatile u32)cmp) \
                                 :"m"(dest), "r2"(result) \
                                 :"r3", "r4"); \
        } while(0); \
    }
