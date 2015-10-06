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

#include "../mm/mm.h"

bool check_str_arg(char* arg, size_t max_len)
{
	char* p;
	size_t count;
	u32 page;

	if(!mm_virt_test(arg, 1, PG_STAT_USER | PG_STAT_COMMIT)) {
		return false;
	}

	for(count = 0, p = arg, page = (u32)arg / PAGE_SIZE;
	    count <= max_len; count++, p++) {
		if(page < (u32)p / PAGE_SIZE) {
			page = (u32)p / PAGE_SIZE;

			if(!mm_virt_test(p, 1, PG_STAT_USER | PG_STAT_COMMIT)) {
				return false;
			}
		}

		if(*p == '\0') {
			return true;
		}
	}

	return false;
}
