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

#include "../../../../../../common/common.h"

#include "../../../mm/mm_defs.h"

typedef struct _heap_t	heap_t, *pheap_t;

typedef	struct	_array_blk {
    size_t		ref;
    void**		p_p_items;
} array_blk_t, *parray_blk_t;

#include "../container_defs.h"

typedef	struct	_array {
    size_t			size;
    size_t			scale;
    pheap_t			heap;
    parray_blk_t*	p_p_blks;
} array_t, *parray_t;
