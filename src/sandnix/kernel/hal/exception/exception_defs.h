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
#include "errno.h"

#define	RAISE(e) hal_exception_raise(e)

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
