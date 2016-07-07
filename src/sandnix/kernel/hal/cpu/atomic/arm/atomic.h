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


#define hal_cpu_atomic_xaddl(dest, src) { \
        do { \
            __asm__ __volatile ( \
                                 "_XADD:\n" \
                                 "ldrex	r0, [%1]\n" \
                                 "mov	r1, r0\n" \
                                 "add	r1, %0, r0\n" \
                                 "strex	r2, r0, [%1]\n" \
                                 "teq	r2, #0\n" \
                                 "bne	_XADD\n" \
                                 "mov	%0, r1\n" \
                                 :"+r"((src)) \
                                 :"r"(&(dest)) \
                                 :"memory", "r0", "r1", "r2"); \
        } while(0); \
    }


#define hal_cpu_atomic_cmpmovl(dest, src, cmp, result) { \
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
