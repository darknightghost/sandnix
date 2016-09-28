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

#include "../../../hal/exception/exception.h"

#include "../../rtl/rtl.h"
#include "../../mm/mm.h"
#include "../../pm/pm.h"

#include "./except_obj.h"
#include "../exception.h"

static	void			raise(pexcept_obj_t p_this);
static	void			destructor(pexcept_obj_t p_this);
static	int				compare(pexcept_obj_t p_this, pexcept_obj_t p_str);
static	pkstring_obj_t	to_string(pexcept_obj_t p_this);

static	pheap_t	p_except_obj_heap = NULL;
static	u8		except_obj_heap_buf[4096];

pexcept_obj_t except_obj(size_t size, kstatus_t reason, pcontext_t p_context)
{
    if(p_except_obj_heap == NULL) {
        //Create heap
        p_except_obj_heap = core_mm_heap_create_on_buf(HEAP_PREALLOC | HEAP_MULITHREAD,
                            4096,
                            except_obj_heap_buf,
                            sizeof(except_obj_heap_buf));

        if(p_except_obj_heap == NULL) {
            PANIC(ENOMEM, "Failed to create heap for exception objects.\n");
        }
    }

    //Allocate memory
    pexcept_obj_t p_ret = (pexcept_obj_t)obj(CLASS_EXCEPT_OBJ,
                          (destructor_t)destructor,
                          (compare_obj_t)compare,
                          (to_string_t)to_string,
                          p_except_obj_heap, size);

    if(p_ret == NULL) {
        PANIC(ENOMEM, "Failed to create exception object.\n");
    }

    //Initialize exception object
    p_ret->p_context = p_context;
    p_ret->reason = reason;
    p_ret->raise = raise;

    return p_ret;
}

void raise(pexcept_obj_t p_this)
{
    //TODO:
    UNREFERRED_PARAMETER(p_this);
}

void destructor(pexcept_obj_t p_this)
{
    core_mm_heap_free(p_this, p_this->obj.heap);
    return;
}

int compare(pexcept_obj_t p_this, pexcept_obj_t p_1)
{
    return p_this->reason - p_1->reason;
}

pkstring_obj_t to_string(pexcept_obj_t p_this)
{
    pkstring_obj_t p_ret = kstring(hal_exception_get_err_name(p_this->reason),
                                   p_except_obj_heap);
    return p_ret;
}
