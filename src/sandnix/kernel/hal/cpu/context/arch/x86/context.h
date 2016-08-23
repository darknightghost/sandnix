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
    } environment;
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
    u32			edi;
    u32			esi;
    u32			ebx;
    u32			edx;
    u32			ecx;
    u32			eax;
    u32			esp;
    u32			ebp;
    u32			eflags;
    u32			eip;
    u16			es;
    u16			cs;
    u16			ss;
    u16			ds;
    u16			fs;
    u16			gs;
    fpu_env_t	fpu_env;
} __attribute__((packed)) context_t, *pcontext_t;
