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

#include "../../interrupt.h"

#include "../../../../init/init_defs.h"

typedef	struct _tss {
    u16		prev_task_link;
    u16		reserved0;
    u32		esp0;
    u16		ss0;
    u16		reserved1;
    u32		esp1;
    u16		ss1;
    u16		reserved2;
    u32		esp2;
    u16		ss2;
    u16		reserved3;
    u32		cr3;
    u32		eip;
    u32		eflags;
    u32		eax;
    u32		ecx;
    u32		edx;
    u32		ebx;
    u32		esp;
    u32		ebp;
    u32		esi;
    u32		edi;
    u16		es;
    u16		reserved4;
    u16		cs;
    u16		reserved5;
    u16		ss;
    u16		reserved6;
    u16		ds;
    u16		received7;
    u16		fs;
    u16		received8;
    u16		gs;
    u16		received9;
    u16		ldt_selector;
    u16		reserved10;
    u16		trap: 1;
    u16		reserved: 7;
    u16		io_map_base_addr;
} __attribute__((packed)) tss_t, *ptss_t;
