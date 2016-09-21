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
#include "../../mm/mm.h"
#include "class_ids.h"

struct	_obj;
struct	_kstring_obj;

//kstring_obj_t obj_t.to_string(pobj_t p_this);
typedef	struct _kstring_obj*	(*to_string_t)(struct _obj*);

//int obj_t.compare(pobj_t p_this, pobj_t p_obj2);
typedef	int	(*compare_obj_t)(struct _obj*, struct _obj*);

//void obj_t.destructor(pobj_t p_this);
typedef	void (*destructor_t)(struct _obj*);

typedef	struct	_obj {
    u32				class_id;
    u32				ref_count;
    pheap_t			heap;
    destructor_t	destructor;
    compare_obj_t	compare;
    to_string_t		to_string;
} obj_t, *pobj_t;

#include "../kstring/kstring.h"

//Increase reference count
#define INC_REF(p_obj)				(core_rtl_obj_inc_ref((pobj_t)(p_obj)))

//Decrease reference count
#define DEC_REF(p_obj)				(core_rtl_obj_dec_ref((pobj_t)(p_obj)))

//To string
#define TO_STRING(p_obj)			(((pobj_t)(p_obj))->to_string( \
                                     (pobj_t)(p_obj)))

//class id
#define	CLASS_ID(p_obj)				(*(((pobj_t)(p_obj))->class_id))

//Constructor
pobj_t		obj(u32 class_id, destructor_t destructor,
                compare_obj_t compare_func, to_string_t to_string_func,
                pheap_t heap, size_t size);

//Increase reference count
void		core_rtl_obj_inc_ref(pobj_t p_obj);

//Decrease reference count
void		core_rtl_obj_dec_ref(pobj_t p_obj);
