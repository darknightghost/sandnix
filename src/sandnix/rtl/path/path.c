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

#include "../rtl.h"
#include "../../exceptions/exceptions.h"
#include "../../pm/pm.h"

k_status rtl_get_next_name_in_path(char** p_path, char* buf, size_t size)
{
	char* p;
	size_t count;
	char* p_old;

	p_old = *p_path;

	while(**p_path == '/') {
		(*p_path++);
	}

	count = 0;
	p = buf;

	while(**p_path != '/' &&**p_path != '\0') {
		if(count >= size - 1) {
			*p_path = p_old;
			pm_set_errno(ENOMEM);
			return ENOMEM;

		} else {
			*p = **p_path;
		}

		count++;
		(*p_path)++;
		p++;
	}

	*p = '\0';

	pm_set_errno(ESUCCESS);
	return ESUCCESS;
}
