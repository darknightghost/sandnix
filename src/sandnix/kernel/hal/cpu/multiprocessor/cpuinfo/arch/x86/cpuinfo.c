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

#include "../../../../../../core/rtl/rtl.h"
#include "../../../../../../core/pm/pm.h"
#include "../../../../../../core/mm/mm.h"
#include "../../../../../io/io.h"
#include "../../../../../exception/exception.h"
#include "../../cpuinfo.h"

static	bool			initialized = false;
static	cpu_id_info_t	cpu_id_infos[MAX_CPU_NUM] = {0};

#define	MODULE_NAME hal_cpu

void PRIVATE(cpuinfo_init)()
{
    pcpu_id_info_t p_info = core_mm_heap_alloc(sizeof(cpu_id_info_t),
                            NULL);

    if(p_info == NULL) {
        PANIC(ENOMEM, "Failed to allocate memory for cpu0.\n");
    }

    p_info = &cpu_id_infos[0];
    p_info->used = true;
    p_info->index = 0;
    p_info->cpuid = hal_cpu_get_cpu_id();

    initialized = true;

    return;
}

void PRIVATE(cpu_id_info_core_init)();
void PRIVATE(cpu_id_info_core_release)();

u32	hal_cpu_get_cpu_id()
{
    return hal_io_apic_read32(LOCAL_APIC_ID_REG);
}

u32	hal_cpu_get_cpu_index()
{
    if(!initialized) {
        return 0;
    }

    u32 cpuid = hal_cpu_get_cpu_id();

    //Look for index
    for(u32 i = 0; i < MAX_CPU_NUM; i++) {
        if(cpu_id_infos[i].used
           && cpu_id_infos[i].cpuid == cpuid) {

            return i;
        }
    }

    return INVALID_CPU_INDEX;
}

u32	hal_cpu_get_cpu_id_by_index(u32 index)
{
    if(index >= MAX_PROCESS_NUM
       || !cpu_id_infos[index].used) {
        return INVALID_CPU_ID;
    }

    return cpu_id_infos[index].cpuid;
}

u32	hal_cpu_get_cpu_index_by_id(u32 id)
{
    if(!initialized) {
        return 0;
    }

    //Look for index
    for(u32 i = 0; i < MAX_CPU_NUM; i++) {
        if(cpu_id_infos[i].used
           && cpu_id_infos[i].cpuid == id) {

            return i;
        }
    }

    return INVALID_CPU_INDEX;
}

void hal_cpu_get_info(pcpuinfo_t p_ret)
{
    UNREFERRED_PARAMETER(p_ret);
}
