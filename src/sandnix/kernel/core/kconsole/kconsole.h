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

#include "../../../../common/common.h"

#include "./kconsole_defs.h"

#define core_kconsole_print_panic(fmt, ...)	core_kconsole_kprint( \
        PRINT_LEVEL_PANIC, \
        fmt, \
        ##__VA_ARGS__);

#define core_kconsole_print_err(fmt, ...) core_kconsole_kprint( \
        PRINT_LEVEL_ERR, \
        fmt, \
        ##__VA_ARGS__);

#define core_kconsole_print_warning(fmt, ...) core_kconsole_kprint( \
        PRINT_LEVEL_WARNING, \
        fmt, \
        ##__VA_ARGS__);

#define core_kconsole_print_info(fmt, ...) core_kconsole_kprint( \
        PRINT_LEVEL_INFO, \
        fmt, \
        ##__VA_ARGS__);

#define core_kconsole_print_debug(fmt, ...)	core_kconsole_kprint( \
        PRINT_LEVEL_DEBUG, \
        fmt, \
        ##__VA_ARGS__);


//Initialize
void core_kconsole_init();

//Print kernel information
void core_kconsole_kprint(u32 level, char* fmt, ...);

//Set output console, NULL is early_print
kstatus_t core_kconsole_set_output(char* output_device);
