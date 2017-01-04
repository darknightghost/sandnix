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
#include "../../core/rtl/rtl.h"

#include "debug.h"
#include "../early_print/early_print.h"
#include "../kparam/kparam.h"
#include "../exception/exception.h"


static	bool	is_debug_on = true;

void hal_debug_init()
{
    char buf[32];
    hal_early_print_printf("\nInitializing debug module..\n");

    if(hal_kparam_get_value("debug", buf, sizeof(buf)) != ESUCCESS
       || core_rtl_strcmp(buf, "on") != 0) {
        hal_early_print_printf("Debug off.\n");
        is_debug_on = false;

    } else {
        hal_early_print_printf("Debug on.\n");
        is_debug_on = true;
    }

    return;
}

bool hal_debug_is_on_dbg()
{
    return is_debug_on;
}
