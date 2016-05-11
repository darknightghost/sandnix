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

#pragma once

#include "../../../common/common.h"

#ifndef	_ASM
#define	INIT_KOBJECT(p_obj,destructor,to_string){ \
		om_init_kobject((pkobject_t)(p_obj), \
		                (kobj_destructor_t)(destructor), \
		                (kobj_to_str)(to_string)); \
	}

#define TO_STRING(p_obj) ((pkstring_t)(((pkobject_t)(p_obj))->to_string(p_obj)))

struct	_kobject;
typedef	void	(*kobj_destructor_t)(struct _kobject*);
typedef	void*	(*kobj_to_str)(struct _kobject*);

typedef	struct _kobject {
	u32					ref_count;
	kobj_destructor_t	destructor;
	kobj_to_str			to_string;
} kobject_t, *pkobject_t;

void	om_init_kobject(pkobject_t p_obj,
                        kobj_destructor_t destructor,
                        kobj_to_str to_string);
void	om_inc_kobject_ref(pkobject_t p_obj);
void	om_dec_kobject_ref(pkobject_t p_obj);
#endif	//!	_ASM
