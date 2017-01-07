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
#include "./exception_defs.h"
#include "errno.h"
#include "../../core/exception/exception_defs.h"
#include "./arch/exception.h"

//Initialize module
void	hal_exception_init();

//Initialize cpu core
void	hal_exception_core_init(
    u32 cpuid);

//Release cpu core
void	hal_exception_core_release(
    u32 cpuid);

void	hal_exception_panic(
    char* file,
    u32 line,
    u32 error_code,		//errno
    char* fmt,			//format
    ...);

//Error code to string. Don't free.
const char*	hal_exception_get_err_name(kstatus_t error_code);

//Raise ecxeption
void hal_exception_raise(pexcept_obj_t p_exception, char* file, u32 line,
                         char* comment);

#define	RAISE(e, comment) hal_exception_raise((e),__FILE__,__LINE__,(comment))

#define	PANIC(error_code, fmt, ...)	hal_exception_panic(__FILE__, __LINE__, \
        (error_code), (fmt), ##__VA_ARGS__);

#include "../debug/debug.h"

#define	ASSERT(exp)	({ \
        if(DEBUG) { \
            if(!(exp)) { \
                PANIC(EASSERT, \
                      "Assert failed, expression \"%s\" is false.", \
                      #exp); \
            } \
        })

#define	NOT_SUPPORT		PANIC(ENOTSUP,"Function not completed!")
