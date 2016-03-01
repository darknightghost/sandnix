/*
	  Copyright 2016,暗夜幽灵 <darknightghost.cn@gmail.com>

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

#include "om.h"
#include "../rtl/rtl.h"

void om_init_kobject(pkobject_t p_obj, kobj_destructor_t destructor
                     , kobj_to_str to_string)
{
	p_obj->ref_count = 1;
	p_obj->destructor = destructor;
	p_obj->to_string = to_string;
	return;
}

void om_inc_kobject_ref(pkobject_t p_obj)
{
	(p_obj->ref_count)++;
	return;
}

void om_dec_kobject_ref(pkobject_t p_obj)
{
	(p_obj->ref_count)--;

	if(p_obj->ref_count == 0) {
		p_obj->destructor(p_obj);
	}

	return;
}
