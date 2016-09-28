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
#include "../obj/obj.h"

typedef struct	_kstring_obj {
    obj_t		obj;
    char*		buf;

    //Methods
    //Get length
    //size_t kstring_obj.len(pkstring_obj_t p_this);
    size_t	(*len)(struct _kstring_obj*);

    //Copy
    //pkstring_obj_t kstring_obj.copy(pkstring_obj_t p_this, pheap_t heap);
    struct _kstring_obj*	(*copy)(struct _kstring_obj*, pheap_t);

    //Get sub string
    //pkstring_obj_t kstring_obj.substr(pkstring_obj_t p_this,u32 begin,u32 end)
    struct  _kstring_obj*	(*substr)(struct _kstring_obj*, u32, u32);

    //Contract string
    //pkstring_obj_t kstring_obj.append(pkstring_obj_t p_this, pktring_obj_t p_str);
    struct _kstring_obj*	(*append)(struct _kstring_obj*, struct _kstring_obj*);

    //Set all characters uppercase.
    //pkstring_obj_t kstring_obj.upper(pkstring_obj_t p_this);
    struct _kstring_obj*	(*upper)(struct _kstring_obj*);

    //Set all characters lowercase.
    //pkstring_obj_t kstring_obj.lower(pkstring_obj_t p_this);
    struct _kstring_obj*	(*lower)(struct _kstring_obj*);

    //Search the beging position of sub string.
    //int kstring_obj.search(pkstring_obj_t p_this, pkstring_obj_t p_substr);
    int	(*search)(struct _kstring_obj*, struct _kstring_obj*);
} kstring_obj_t, *pkstring_obj_t;

//Constructor
pkstring_obj_t kstring(const char* str, pheap_t heap);
