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

u64 rtl_div64(u64 dividend, u32 divisor)
{
	u64 ret;
	u8 i;

#pragma pack(1)
	union {
		u64	value;
		struct {
			u32 low;
			u32 high;
		} part;
	} tmp;
#pragma pack()

	if((dividend >> 32) == 0) {
		return (u32)dividend / divisor;
	}

	ret = 0;
	tmp.value = dividend;

	for(i = 0; i < 32; i++) {
		ret += tmp.part.high / divisor;
		tmp.part.high = tmp.part.high % divisor;
		ret = ret << 1;
		tmp.part.high = tmp.part.high << 1;
	}

	ret += tmp.part.high / divisor;
	tmp.part.high = tmp.part.high % divisor;

	return ret;
}


u64 rtl_mod64(u64 dividend, u32 divisor)
{
	u64 ret;
	u8 i;

#pragma pack(1)
	union {
		u64	value;
		struct {
			u32 low;
			u32 high;
		} part;
	} tmp;
#pragma pack()

	if((dividend >> 32) == 0) {
		return (u32)dividend % divisor;
	}

	ret = 0;
	tmp.value = dividend;

	for(i = 0; i < 32; i++) {
		ret += tmp.part.high / divisor;
		tmp.part.high = tmp.part.high % divisor;
		ret = ret << 1;
		tmp.part.high = tmp.part.high << 1;
	}

	ret += tmp.part.high / divisor;
	tmp.part.high = tmp.part.high % divisor;

	return tmp.part.high;
}

double rtl_sqrt(double num)
{

}
