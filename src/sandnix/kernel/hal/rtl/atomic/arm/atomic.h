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

#define	hal_rtl_atomic_addl(dest, num) { \
        __asm__ __volatile__( \
                              "1:\n" \
                              "ldrex	r0, [%0]\n"  \
                              "add		r0, %1\n" \
                              "strex	r1, r0, [%0]\n" \
                              "teq	r1, #0\n" \
                              "bne	1b\n" \
                              ::"r"(&(dest)), "r"((num)) \
                              :"r0", "r1", "memory"); \
    }


#define	hal_rtl_atomic_subl(dest, num) { \
        __asm__ __volatile__( \
                              "1:\n" \
                              "ldrex	r0, [%0]\n"  \
                              "sub		r0, %1\n" \
                              "strex	r1, r0, [%0]\n" \
                              "teq	r1, #0\n" \
                              "bne	1b\n" \
                              ::"r"(&(dest)), "r"((num)) \
                              :"r0", "r1", "memory"); \
    }


#define hal_rtl_atomic_xaddl(dest, src) { \
        do { \
            __asm__ __volatile ( \
                                 "1:\n" \
                                 "ldrex	r0, [%1]\n" \
                                 "mov	r1, r0\n" \
                                 "add	r0, %0, r1\n" \
                                 "strex	r2, r0, [%1]\n" \
                                 "teq	r2, #0\n" \
                                 "bne	1b\n" \
                                 "mov	%0, r1\n" \
                                 :"+r"((src)) \
                                 :"r"(&(dest)) \
                                 :"memory", "r0", "r1", "r2"); \
        } while(0); \
    }

#define hal_rtl_atomic_xaddw(dest, src) { \
        do { \
            __asm__ __volatile ( \
                                 "1:\n" \
                                 "ldrexh	r0, [%1]\n" \
                                 "mov		r1, r0\n" \
                                 "add		r0, %0, r1\n" \
                                 "strexh	r2, r0, [%1]\n" \
                                 "teq		r2, #0\n" \
                                 "bne		1b\n" \
                                 "mov		%0, r1\n" \
                                 :"+r"((src)) \
                                 :"r"(&(dest)) \
                                 :"memory", "r0", "r1", "r2"); \
        } while(0); \
    }

#define hal_rtl_atomic_cmpxchgl(dest, src, cmp, result) { \
        do { \
            __asm__ __volatile__ ( \
                                   "ldrex	r0, [%2]\n" \
                                   "cmp		r0, %3\n" \
                                   "movne	%1, #0\n" \
                                   "bne		1f\n" \
                                   "strex	r1, %0, [%2]\n" \
                                   "teq		r1, #0\n" \
                                   "moveq	%1, #0\n" \
                                   "movne	%1, #1\n" \
                                   "1:\n" \
                                   "clrex\n" \
                                   :"+r"((src)), "=r"(result) \
                                   :"r"(&(dest)), "r"((cmp)) \
                                   :"r0", "r1", "memory"); \
        } while(0); \
    }
