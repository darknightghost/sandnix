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

#include "../../../../common/common.h"
#include "./heap/heap.h"
#include "./paging/paging.h"

#include "./mm.h"

void core_mm_init()
{
    core_mm_paging_init();
    return;
}

void core_mm_core_init(u32 cpuid)
{
    core_mm_paging_cpu_core_init(cpuid);
    return;
}

void core_mm_core_release(u32 cpuid)
{
    core_mm_paging_cpu_core_release(cpuid);
    return;
}
