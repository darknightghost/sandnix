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

#include "../../cpuinfo.h"
#include "../../../../../io/io.h"
#include "../../../../../exception/exception.h"
#include "../../../../../../core/rtl/rtl.h"
#include "../../../../../../core/pm/pm.h"
#include "../../../../../../core/mm/mm.h"

void cpuinfo_init()
{
}

void cpu_id_info_core_init();
void cpu_id_info_core_release();

u32	hal_cpu_get_cpu_id()
{
}

u32	hal_cpu_get_cpu_index()
{

}

u32	hal_cpu_get_cpu_id_by_index(u32 index)
{

}

u32	hal_cpu_get_cpu_index_by_id(u32 id)
{

}

void hal_cpu_get_info(pcpuinfo_t p_ret)
{
    UNREFERRED_PARAMETER(p_ret);
}

