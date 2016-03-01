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

#pragma once

#include "../../../../common/common.h"
#include "../../om/om.h"
#include "string.h"
#include "../rtl.h"

typedef struct _kstring {
	kobject_t	obj;
	char*		buf;
	void*		heap;
	size_t		len;
} kstring_t, *pkstring_t;

pkstring_t	rtl_kstring(char* string, void* heap);
pkstring_t	rtl_kstrcat(pkstring_t str1, pkstring_t str2, void* heap);
size_t		rtl_kstrlen(pkstring_t str);
ssize_t		rtl_find_sub_kstr(pkstring_t str, pkstring_t substr);
pkstring_t	rtl_is_sub_kstr(pkstring_t str, pkstring_t substr);

