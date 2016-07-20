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

#include "thread.h"

static int tmp_priority = PRIORITY_DISPATCH;

u32 core_pm_get_crrnt_thread_id()
{
    return 0;
}

u32 core_pm_get_thrd_priority(u32 thrd_id)
{
    UNREFERRED_PARAMETER(thrd_id);
    return tmp_priority;
}

void core_pm_set_thrd_priority(u32 thrd_id, u32 priority)
{
    UNREFERRED_PARAMETER(thrd_id);
    tmp_priority = priority;
    return;
}
