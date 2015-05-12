/*
	Copyright 2015,暗夜幽灵 <darknightghost.cn@gmail.com>

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

#include "../../math.h"

u64 div64(u64 dividend, u32 divisor)
{
	u64	ret = 0;

	u32 mid;

	if((dividend >> 32) == 0) {
		return	(u32)dividend / divisor;
	}

	return 0;
}

u64 mod64(u64 dividend, u32 divisor)
{
	return 0;
}

