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

#include "./efloat_except.h"
#include "../exception.h"

pefloat_except_t efloat_except()
{
    pefloat_except_t p_ret = (pefloat_except_t)except_obj(sizeof(efloat_except_t),
                             EFLOAT);

    if(p_ret != NULL) {
        p_ret->except.obj.class_id = CLASS_EXCEPT(EFLOAT);
    }

    return p_ret;
}
