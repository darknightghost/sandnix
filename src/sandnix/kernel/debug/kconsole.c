/*
	  Copyright 2016,暗夜幽灵 <darknightghost.cn@gmail.com>

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

#include "../../../common/common.h"
#include "../pm/pm.h"
#include "../rtl/rtl.h"
#include "kconsole.h"
#include "early_print.h"

static	spinlock_t	print_lock;
static	char		kprint_buf[1024];

void kconsole_init()
{
	pm_init_spn_lock(&print_lock);
	early_cls();
	return;
}

u32 dbg_kprint(char* fmt, ...)
{
	va_list args;
	u32 ret;

	pm_acqr_spn_lock(&print_lock);

	va_start(args, fmt);
	ret = rtl_vnprintf(kprint_buf, 1024, fmt, args);
	va_end(args);

	early_print(kprint_buf);

	pm_rls_spn_lock(&print_lock);
	return ret;
}
