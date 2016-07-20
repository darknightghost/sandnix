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

#include "../../../../../common/common.h"

#if defined(X86) || defined(ARM)
u64		hal_rtl_math_div64(u64 dividend, u32 divisor);
u64		hal_rtl_math_mod64(u64 dividend, u32 divisor);

#else
#define		hal_rtl_math_div64(dividend, divisor)	((dividend) / (divisor))
#define		hal_rtl_math_mod64(dividend, divisor)	((dividend) % (divisor))
#endif
