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

#include "../../../../../../common/common.h"

#define hal_rtl_string_movsb(dest, src, count) { \
        do {\
            __asm__ __volatile__( \
                                  "1:\n" \
                                  "ldrb	r0, [%1]\n" \
                                  "strb	r0, [%0]\n" \
                                  "add	%1, %1, #1\n" \
                                  "add	%0, %0, #1\n" \
                                  "sub	%2, %2, #1\n" \
                                  "teq	%2, #0\n" \
                                  "bne	1b\n" \
                                  ::"r"((u32)(dest)), "r"((u32)(src)), \
                                  "r"((u32)(count)) \
                                  :"memory", "r0"); \
        } while(0); \
    }

#define hal_rtl_string_movsw(dest, src, count) { \
        do {\
            __asm__ __volatile__( \
                                  "1:\n" \
                                  "ldrh	r0, [%1]\n" \
                                  "strh	r0, [%0]\n" \
                                  "add	%1, %1, #2\n" \
                                  "add	%0, %0, #2\n" \
                                  "sub	%2, %2, #1\n" \
                                  "teq	%2, #0\n" \
                                  "bne	1b\n" \
                                  ::"r"((u32)(dest)), "r"((u32)(src)), \
                                  "r"((u32)(count)) \
                                  :"memory", "r0"); \
        } while(0); \
    }


#define hal_rtl_string_movsl(dest, src, count) { \
        do {\
            __asm__ __volatile__( \
                                  "1:\n" \
                                  "ldr	r0, [%1]\n" \
                                  "str	r0, [%0]\n" \
                                  "add	%1, %1, #4\n" \
                                  "add	%0, %0, #4\n" \
                                  "sub	%2, %2, #1\n" \
                                  "teq	%2, #0\n" \
                                  "bne	1b\n" \
                                  ::"r"((u32)(dest)), "r"((u32)(src)), \
                                  "r"((u32)(count)) \
                                  :"memory", "r0"); \
        } while(0); \
    }

#define hal_rtl_string_movsq(dest, src, count) { \
        do {\
            __asm__ __volatile__( \
                                  "1:\n" \
                                  "ldr	r0, [%1]\n" \
                                  "str	r0, [%0]\n" \
                                  "add	%1, %1, #8\n" \
                                  "add	%0, %0, #8\n" \
                                  "sub	%2, %2, #1\n" \
                                  "teq	%2, #0\n" \
                                  "bne	1b\n" \
                                  ::"r"((u32)(dest)), "r"((u32)(src)), \
                                  "r"((u32)(count * 2)) \
                                  :"memory","r0"); \
        } while(0); \
    }

#define hal_rtl_string_movsb_back(dest, src, count) { \
        do {\
            __asm__ __volatile__( \
                                  "1:\n" \
                                  "ldrb	r0, [%1]\n" \
                                  "strb	r0, [%0]\n" \
                                  "sub	%1, %1, #1\n" \
                                  "sub	%0, %0, #1\n" \
                                  "sub	%2, %2, #1\n" \
                                  "teq	%2, #0\n" \
                                  "bne	1b\n" \
                                  ::"r"((u32)(dest) + (u32)(count) - 1), \
                                  "r"((u32)(src) + (u32)(count) - 1), \
                                  "r"((u32)(count)) \
                                  :"r0","memory"); \
        } while(0); \
    }

#define hal_rtl_string_movsw_back(dest, src, count) { \
        do {\
            __asm__ __volatile__( \
                                  "1:\n" \
                                  "ldrh	r0, [%1]\n" \
                                  "strh	r0, [%0]\n" \
                                  "sub	%1, %1, #2\n" \
                                  "sub	%0, %0, #2\n" \
                                  "sub	%2, %2, #1\n" \
                                  "teq	%2, #0\n" \
                                  "bne	1b\n" \
                                  ::"r"((u32)(dest) + (u32)(count) * 2 - 2), \
                                  "r"((u32)(src) + (u32)(count) * 2 - 2), \
                                  "r"((u32)(count)) \
                                  :"r0","memory"); \
        } while(0); \
    }


#define hal_rtl_string_movsl_back(dest, src, count) { \
        do {\
            __asm__ __volatile__( \
                                  "1:\n" \
                                  "ldr	r0, [%1]\n" \
                                  "str	r0, [%0]\n" \
                                  "sub	%1, %1, #4\n" \
                                  "sub	%0, %0, #4\n" \
                                  "sub	%2, %2, #1\n" \
                                  "teq	%2, #0\n" \
                                  "bne	1b\n" \
                                  ::"r"((u32)(dest) + (u32)(count) * 4 - 4), \
                                  "r"((u32)(src) + (u32)(count) * 4 - 4), \
                                  "r"((u32)(count)) \
                                  :"r0","memory"); \
        } while(0); \
    }

#define hal_rtl_string_movsq_back(dest, src, count) { \
        do {\
            __asm__ __volatile__( \
                                  "1:\n" \
                                  "ldr	r0, [%1]\n" \
                                  "str	r0, [%0]\n" \
                                  "sub	%1, %1, #8\n" \
                                  "sub	%0, %0, #8\n" \
                                  "sub	%2, %2, #1\n" \
                                  "teq	%2, #0\n" \
                                  "bne	1b\n" \
                                  ::"r"((u32)(dest) + (u32)(count) * 4 * 2 - 4), \
                                  "r"((u32)(src) + (u32)(count) * 4 * 2 - 4), \
                                  "r"((u32)(count) * 2) \
                                  :"r0","memory"); \
        } while(0); \
    }

#define	hal_rtl_string_setsb(dest, val, count)	{ \
        do { \
            __asm__ __volatile__( \
                                  "1:\n" \
                                  "strb	%1, [%0]\n" \
                                  "add	%0, %0, #1\n" \
                                  "sub	%2, %2, #1\n" \
                                  "teq	%2, #0\n" \
                                  "bne	1b\n" \
                                  ::"r"((dest)), "r"((u8)(val)), \
                                  "r"((u32)(count)) \
                                  :"memory"); \
        } while(0); \
    }

#define	hal_rtl_string_setsw(dest, val, count)	{ \
        do { \
            __asm__ __volatile__( \
                                  "1:\n" \
                                  "strh	%1, [%0]\n" \
                                  "add	%0, %0, #2\n" \
                                  "sub	%2, %2, #1\n" \
                                  "teq	%2, #0\n" \
                                  "bne	1b\n" \
                                  ::"r"((dest)), "r"((u16)(val) | ((u16)val << 8)), \
                                  "r"((u32)(count)) \
                                  :"memory"); \
        } while(0); \
    }

#define	hal_rtl_string_setsl(dest, val, count)	{ \
        do { \
            __asm__ __volatile__( \
                                  "1:\n" \
                                  "str	%1, [%0]\n" \
                                  "add	%0, %0, #4\n" \
                                  "sub	%2, %2, #1\n" \
                                  "teq	%2, #0\n" \
                                  "bne	1b\n" \
                                  ::"r"((dest)), \
                                  "r"((u32)(val) | ((u32)val << 8) \
                                      | ((u32)val << 16)), \
                                  "r"((u32)(count)) \
                                  :"memory"); \
        } while(0); \
    }

#define	hal_rtl_string_setsq(dest, val, count)	{ \
        do { \
            __asm__ __volatile__( \
                                  "1:\n" \
                                  "str	%1, [%0]\n" \
                                  "add	%0, %0, #8\n" \
                                  "sub	%2, %2, #1\n" \
                                  "teq	%2, #0\n" \
                                  "bne	1b\n" \
                                  ::"r"((dest)), \
                                  "r"((u32)(val) | ((u32)val << 8) \
                                      | ((u32)val << 16)), \
                                  "r"((u32)(count) * 2) \
                                  :"memory"); \
        } while(0); \
    }
