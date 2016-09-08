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

#include "../../id.h"
#include "../../../../../io/io.h"
#include "../../../../../../core/rtl/rtl.h"
#include "../../../../../../core/pm/pm.h"
#include "../../../../../../core/mm/mm.h"

static	bool		initialized = false;

void cpu_id_init()
{
    return;
}

u32	hal_cpu_get_cpu_id()
{
    return hal_io_apic_read32(LOCAL_APIC_ID_REG);
}

u32	hal_cpu_get_cpu_index()
{
    if(!initialized) {
        return 0;
    }
}

u32	hal_cpu_get_cpu_id_by_index(u32 index);
u32	hal_cpu_get_cpu_index_by_id(u32 id);
