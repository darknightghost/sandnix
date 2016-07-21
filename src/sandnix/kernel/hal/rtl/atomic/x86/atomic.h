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

#define hal_rtl_atomic_xaddl(dest, src) { \
        do { \
            __asm__ __volatile__( \
                                  "lock		xaddl %0, %1\n" \
                                  :"+eax"((src)), "+m"((dest)) \
                                  :: "memory");\
        } while(0); \
    }

#define hal_rtl_atomic_xaddw(dest, src) { \
        do { \
            __asm__ __volatile__( \
                                  "lock		xaddw %0, %1\n" \
                                  :"+ax"((src)), "+m"((dest)) \
                                  :: "memory");\
        } while(0); \
    }

#define hal_rtl_atomic_cmpxchgl(dest, src, cmp, result) { \
        do { \
            __asm__ __volatile__ ( \
                                   "lock cmpxchgl	%2, %0\n" \
                                   "jz				_EQUAL\n" \
                                   "xorl			%1, %1\n" \
                                   "jmp				_END\n" \
                                   "_EQUAL:\n" \
                                   "movl			$1, %1\n" \
                                   "_END:\n" \
                                   :"=m"(dest), "=r"(result), "+r"(src) \
                                   :"eax"(cmp) \
                                   :"memory"); \
        } while(0); \
    }
