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

#include "../../rtl/rtl_defs.h"

#include "../../../hal/cpu/cpu_defs.h"

struct	_except_obj;
//void	raise(pexcept_obj_t p_this, pcontext_t p_context, char* file,
//	u32 line, char* comment);
typedef	void	(*except_raise_t)(struct _except_obj*, pcontext_t, char*, u32,
                                  char*);

//void	panic(pexcept_obj_t p_this);
typedef void	(*except_panic_t)(struct _except_obj*);

typedef	struct	_except_obj {
    obj_t			obj;
    kstatus_t		reason;
    pcontext_t		p_context;
    pkstring_obj_t	file;
    u32				line;
    pkstring_obj_t	comment;

    //void	raise(pexcept_obj_t p_this, pcontext_t p_context, char* file,
    //	u32 line, char* comment);
    except_raise_t	raise;

    //void	panic(pexcept_obj_t p_this);
    except_panic_t	panic;
} except_obj_t, *pexcept_obj_t;

#include "./eperm_except_defs.h"
#include "./enoent_except_defs.h"
#include "./ediv_except_defs.h"
#include "./eunknowint_except_defs.h"
#include "./ebreakpoint_except_defs.h"
#include "./eundefined_except_defs.h"
#include "./efloat_except_defs.h"
#include "./eprivilege_except_defs.h"
#include "./epageread_except_defs.h"
#include "./epagewrite_except_defs.h"
#include "./epageexec_except_defs.h"
#include "./edeadlock_except_defs.h"
#include "./epfinpaging_except_defs.h"
#include "./einval_except_defs.h"
#include "./ehpcorruption_except_defs.h"
#include "./enomem_except_defs.h"
#include "./eagain_except_defs.h"
