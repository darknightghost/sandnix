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

#include "../../../../../../../common/common.h"

typedef	struct {
    u8		byte0;
    u8		byte1;
    u8		byte2;
    u8		byte3;
    u8		byte4;
    u8		byte5;
    u8		byte6;
    u8		byte7;
    u8		byte8;
    u8		byte9;
} __attribute__((packed)) fpu_data_reg_t, *pfpu_data_reg_t;

typedef	struct {
    struct {
        u16		control_word;
        u16		undefined0;
        u16		status_word;
        u16		undefined1;
        u16		tag_word;
        u16		undefined2;
        u32		eip;
        u16		cs_selector;
        u16		op_code;
        u32		operand;
        u16		operand_selector;
        u16		undefined3;
    } __attribute__((packed)) environment;
    fpu_data_reg_t		st0;
    fpu_data_reg_t		st1;
    fpu_data_reg_t		st2;
    fpu_data_reg_t		st3;
    fpu_data_reg_t		st4;
    fpu_data_reg_t		st5;
    fpu_data_reg_t		st6;
    fpu_data_reg_t		st7;
} __attribute__((packed)) fpu_env_t, *pfpu_env_t;

typedef	struct	_context {
    //fnsave
    fpu_env_t	fpu_env;

    //mov
    u16			es;
    u16			es_h;
    u16			ds;
    u16			ds_h;
    u16			fs;
    u16			fs_h;
    u16			gs;
    u16			gs_h;
    u16			ss;
    u16			ss_h;
    u32			eflags;

    //pushal
    u16			cs;
    u16			cs_h;
    u32			eip;
    u32			edi;
    u32			esi;
    u32			ebp;
    u32			esp;
    u32			ebx;
    u32			edx;
    u32			ecx;
    u32			eax;
} __attribute__((packed)) context_t, *pcontext_t;

#define	hal_cpu_context_load(p_context)	{ \
        if((p_context)->ss == SELECTOR_K_DATA) { \
            /* Return to kernel memory*/ \
            /* EIP */ \
            *((u32*)(p_context) + sizeof(context_t) / 4 + 1) = (p_context)->eip; \
            /* EFLAGS */ \
            *((u32*)(p_context) + sizeof(context_t) / 4 + 3) = (p_context)->eflags; \
            \
        } else { \
            /* Return to user memory */ \
            /* EIP */ \
            *((u32*)(p_context) + sizeof(context_t) / 4 + 1) = (p_context)->eip; \
            /* EFLAGS */ \
            *((u32*)(p_context) + sizeof(context_t) / 4 + 3) = (p_context)->eflags; \
            /* ESP */ \
            *((u32*)(p_context) + sizeof(context_t) / 4 + 4) = (p_context)->esp; \
            (p_context)->esp = (u32)((u32*)(p_context) + sizeof(context_t) / 4 + 1); \
        } \
        \
        __asm__ __volatile__( \
                              "movl		%0, %%esp\n" \
                              "frstor	(%%esp)\n" \
                              "addl		%1, %%esp\n" \
                              "addl		$32, %%esp\n" \
                              "popal\n" \
                              "addl		$4, %%esp\n" \
                              "iret\n" \
                              ::"r"(p_context), \
                              "i"(sizeof(fpu_env_t))); \
    }

#define hal_cpu_context_save_call(dest_func)

#define	hal_cpu_get_stack_base(buff, size)	((void*)((address_t)(buff) + (size)))
