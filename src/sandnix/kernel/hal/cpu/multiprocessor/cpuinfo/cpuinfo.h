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
#include "../../../../../../common/common.h"

#if defined X86
    #include "./arch/x86/cpuinfo.h"
#endif

#define INVALID_CPU_INDEX	0xFFFFFFFF
#define INVALID_CPU_ID		0xFFFFFFFF

#if defined X86
    void cpuinfo_init();
    void cpuinfo_core_init();
    void cpuinfo_core_release();
#elif define ARM
    #define	cpuinfo_init()
    #define	cpuinfo_core_init()
    #define	cpuinfo_core_release()
#endif

//Get current CPU id
u32	hal_cpu_get_cpu_id();

//Get current CPU index
u32	hal_cpu_get_cpu_index();

//Get CPU id
u32	hal_cpu_get_cpu_id_by_index(u32 index);

//Get CPU index
u32	hal_cpu_get_cpu_index_by_id(u32 id);

//Get CPU information
void hal_cpu_get_info(pcpuinfo_t p_ret);
