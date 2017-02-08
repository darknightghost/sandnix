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

typedef struct _spnlck_t {
    union {
        volatile u32		lock;
        struct {
            volatile u16		ticket;
            volatile u16		owner;
        } __attribute__((aligned(1)));
    };
    volatile u32		priority;
    volatile u32		cpu_index;
} spnlck_t, *pspnlck_t;
