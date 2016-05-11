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

#include "bitmap.h"

bool rtl_bitmap_read(pbitmap_t p_bitmap, size_t bit)
{
	u8* p_bit;

	p_bit = p_bitmap + bit / 8;

	if((*p_bit) & (1 << (bit % 8))) {
		return true;

	} else {
		return false;
	}
}

void rtl_bitmap_write(pbitmap_t p_bitmap, size_t bit, bool value)
{
	u8* p_bit;

	p_bit = p_bitmap + bit / 8;

	if(value) {
		*p_bit = (*p_bit) | (1 << (bit % 8));

	} else {
		*p_bit = (*p_bit) & (~(1 << (bit % 8)));
	}

	return;
}
