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

#include "./enoent_except.h"
#include "../exception.h"

static	pkstring_obj_t		to_string(penoent_except_t p_this);
static	void				destructor(penoent_except_t p_this);

penoent_except_t enoent_except(pkstring_obj_t p_path)
{
    penoent_except_t p_ret = (penoent_except_t)except_obj(sizeof(enoent_except),
                             ENOENT);

    if(p_ret != NULL) {
        p_ret->path = p_path->copy(p_path, p_except_obj_heap);

        if(p_ret->path == NULL) {
            core_mm_heap_free(p_ret, p_ret->except.obj.heap);
        }

        p_ret->except.obj.class_id = CLASS_EXCEPT(ENOENT);
        p_ret->except.obj.destructor = (destructor_t)destructor;
        p_ret->except.obj.to_string = (to_string_t)to_string;
    }

    return p_ret;
}

pkstring_obj_t to_string(penoent_except_t p_this)
{
    pkstring_obj_t p1 = kstring(
                            "No such file or directory.\nPath : \"",
                            p_except_obj_heap);

    if(p1 == NULL) {
        return NULL;
    }

    pkstring_obj_t p2 = p1->append(p1, p_this->path);

    if(p2 == NULL) {
        DEC_REF(p1);
        return NULL;
    }

    DEC_REF(p1);
    p1 = kstring("\"", p_except_obj_heap);

    if(p1 == NULL) {
        DEC_REF(p2);
        return NULL;
    }

    pkstring_obj_t p_ret = p2->append(p2, p1);
    DEC_REF(p1);
    DEC_REF(p2);

    return p_ret;
}

void destructor(penoent_except_t p_this)
{
    DEC_REF(p_this->path);
    core_mm_heap_free(p_this, p_this->except.obj.heap);
    return;
}
