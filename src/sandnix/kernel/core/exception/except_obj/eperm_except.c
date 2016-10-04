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

#include "eperm_except.h"
#include "../exception.h"

static	pkstring_obj_t		to_string(peperm_except_t p_this);

peperm_except_t eperm_except()
{
    peperm_except_t p_ret = (peperm_except_t)except_obj(
                                sizeof(eperm_except),
                                EPERM);

    if(p_ret != NULL) {
        p_ret->except.obj.class_id = CLASS_EXCEPT(EPERM);
        p_ret->except.obj.to_string = (to_string_t)to_string;
    }

    return p_ret;
}

pkstring_obj_t to_string(peperm_except_t p_this)
{
    return kstring("Operation not permitted.", p_except_obj_heap);
    UNREFERRED_PARAMETER(p_this);
}
