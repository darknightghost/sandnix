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

#include "../../../../common/common.h"
#include "../rtl.h"
#include "string.h"
#include "../../mm/mm.h"
#include "../../om/om.h"

static pkstring_t	to_string(pkstring_t p_self);
static void			destructor(pkstring_t p_self);

pkstring_t rtl_kstring(char* string, void* heap)
{
	pkstring_t p_ret;

	p_ret = mm_hp_alloc(sizeof(kstring_t), heap);
	INIT_KOBJECT(p_ret, destructor, to_string);

	p_ret->len = rtl_strlen(string);
	p_ret->buf = mm_hp_alloc(p_ret->len + 1, heap);
	p_ret->heap = heap;

	rtl_strncpy(p_ret->buf, p_ret->len + 1, string);

	return p_ret;
}

pkstring_t rtl_kstrcat(pkstring_t str1, pkstring_t str2, void* heap)
{
	pkstring_t p_ret;

	p_ret = mm_hp_alloc(sizeof(kstring_t), heap);
	INIT_KOBJECT(p_ret, destructor, to_string);

	p_ret->len = str1->len + str2->len;
	p_ret->buf = mm_hp_alloc(p_ret->len + 1, heap);
	p_ret->heap = heap;

	rtl_strncpy(p_ret->buf, p_ret->len + 1, str1->buf);
	rtl_strncat(p_ret->buf, p_ret->len + 1, str2->buf);

	return p_ret;
}

size_t rtl_kstrlen(pkstring_t str)
{
	return str->len;
}

pkstring_t rtl_substr(ssize_t begin, ssize_t end, pkstring_t str, void* heap)
{
	pkstring_t p_ret;

	if(begin < 0 || begin > str->len - 1) {
		return NULL;
	}

	if(end > str->len - 1 || end == -1) {
		end = str->len - 1;
	}

	p_ret = mm_hp_alloc(sizeof(kstring_t), heap);
	INIT_KOBJECT(p_ret, destructor, to_string);

	p_ret->len = end - begin + 1;
	p_ret->buf = mm_hp_alloc(p_ret->len + 1, heap);
	p_ret->heap = heap;

	rtl_memcpy(p_ret->buf, str->buf + begin, p_ret->len);

	*(str->buf + end + 1) = '\0';

	return p_ret;
}

ssize_t rtl_find_sub_kstr(pkstring_t str, pkstring_t substr)
{
	char* p_str;
	char* p_cmp;
	char* p_substr;

	for(p_str = str->buf;
	    p_str - str->buf >= str->len;
	    p_str++) {
		p_substr = substr->buf;
		p_cmp = p_str;

		while(*p_substr == *p_cmp) {
			if(p_substr - substr->buf >= substr->len) {
				return p_str - str->buf;

			} else if(p_cmp - str->buf >= str->len) {
				break;

			} else {
				p_substr++;
				p_cmp++;
			}
		}
	}

	return -1;
}

pkstring_t to_string(pkstring_t p_self)
{
	om_inc_kobject_ref((pkobject_t)p_self);
	return p_self;
}

void destructor(pkstring_t p_self)
{
	mm_hp_free(p_self->buf, p_self->heap);
	mm_hp_free(p_self, p_self->heap);

	return;
}
