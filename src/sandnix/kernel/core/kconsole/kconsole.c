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

#include "../../hal/early_print/early_print.h"
#include "../../hal/debug/debug.h"
#include "../../core/rtl/rtl.h"

#include "kconsole.h"

static	bool	initialized = false;

static	void	do_print(char* p);

void core_kconsole_init()
{
    //TODO:Unfinished
    initialized = true;

    return;
}

void core_kconsole_kprint(u32 level, char* fmt, ...)
{
    char buf[1024];

    if(level > PRINT_LEVEL_INFO
       && !hal_debug_is_on_dbg()) {
        return;
    }

    switch(level) {
        case PRINT_LEVEL_PANIC:
            core_rtl_strncpy(buf, "\x1B[1;31;40m", sizeof(buf));
            break;

        case PRINT_LEVEL_ERR:
            core_rtl_strncpy(buf, "\x1B[1;35;40m", sizeof(buf));
            break;

        case PRINT_LEVEL_WARNING:
            core_rtl_strncpy(buf, "\x1B[1;33;40m", sizeof(buf));
            break;

        case PRINT_LEVEL_INFO:
            core_rtl_strncpy(buf, "\x1B[0m", sizeof(buf));
            break;

        case PRINT_LEVEL_DEBUG:
            core_rtl_strncpy(buf, "\x1B[1;34;40m", sizeof(buf));
            break;
    }

    size_t len = core_rtl_strlen(buf);

    va_list ap;
    va_start(ap, fmt);
    core_rtl_vsnprintf(buf + len, sizeof(buf) - len - 4, fmt, ap);
    core_rtl_strncat(buf, "\x1B[0m", sizeof(buf));
    do_print(buf);
    va_end(ap);

    return;
}

kstatus_t core_kconsole_set_output(char* output_device);

void do_print(char* p)
{
    hal_early_print_puts(p);
    return;
}
