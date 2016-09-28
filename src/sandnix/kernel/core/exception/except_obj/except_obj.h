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

#define CORE_EXCEPTION_EXPORT

#include "../../../../../common/common.h"

#include "../../rtl/rtl.h"

#include "../../../hal/cpu/cpu.h"

typedef	struct	_except_obj {
    obj_t		obj;
    kstatus_t	reason;
    pcontext_t	p_context;

    void	(*raise)(struct _except_obj* p_this);
} except_obj_t, *pexcept_obj_t;

pexcept_obj_t	except_obj(size_t size, kstatus_t reason, pcontext_t p_context);


