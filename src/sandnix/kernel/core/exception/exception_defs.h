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

#include "../../../../common/common.h"

#include "./thread_except_stat_obj_defs.h"
#include "./except_obj/except_obj_defs.h"

#define EXCEPT_STATUS_CONTINUE_EXEC		0x00000000
#define	EXCEPT_STATUS_UNWIND			0x00000001
#define EXCEPT_STATUS_CONTINUE_SEARCH	0x00000002

#define	EXCEPT_REASON_EXCEPT	0x00000000
#define	EXCEPT_REASON_UNWIND	0x00000001

//#define OPERATE_SUCCESS

typedef	u32		except_stat_t;

//except_stat_t except_hndlr_t(u32 reason, pexcept_obj_t p_except, pcontext_t context);
typedef	except_stat_t	(*except_hndlr_t)(u32, pexcept_obj_t, pcontext_t);

typedef	struct	_except_hndlr_info {
    kstatus_t		reason;
    except_hndlr_t	hndlr;
    context_t		context;
} except_hndlr_info_t, *pexcept_hndlr_t;
