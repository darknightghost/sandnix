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

#include "../../../../core/mm/mm.h"
#include "./ipi_arg_obj.h"
#include "../../../../core/rtl/obj/obj.h"

#define	MODULE_NAME hal_cpu
extern	pheap_t					PRIVATE(ipi_queue_heap);

static	pkstring_obj_t	to_string(pipi_arg_obj_t p_this);
static	int				compare(pipi_arg_obj_t p_this, pipi_arg_obj_t p_obj2);
static	void			destructor(pipi_arg_obj_t p_this);

pipi_arg_obj_t ipi_arg_obj(size_t size, u32 class_id, u32 src_cpu)
{
    pipi_arg_obj_t p_ret = (pipi_arg_obj_t)obj(
                               class_id,
                               (destructor_t)destructor,
                               (compare_obj_t)compare,
                               (to_string_t)to_string,
                               PRIVATE(ipi_queue_heap),
                               size);

    if(p_ret != NULL) {
        p_ret->source_cpu = src_cpu;
    }

    return p_ret;
}

pkstring_obj_t to_string(pipi_arg_obj_t p_this)
{
    return kstring("IPI arguments.", p_this->obj.heap);
}

int compare(pipi_arg_obj_t p_this, pipi_arg_obj_t p_obj2)
{
    if((address_t)p_this > (address_t)p_obj2) {
        return 1;

    } else if(p_this == p_obj2) {
        return 0;

    } else {
        return -1;
    }
}

void destructor(pipi_arg_obj_t p_this)
{
    core_mm_heap_free(p_this, p_this->obj.heap);
    return;
}
