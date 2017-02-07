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
#include "./thread_obj.h"
#include "../../rtl/rtl.h"
#include "../../mm/mm.h"
#include "../../exception/exception.h"
#include "../../../hal/cpu/cpu.h"
#include "../../../hal/io/io.h"
#include "../pm.h"

//Heap
static	pheap_t				sched_heap;

//Schedule list
static	list_t				sched_lists[PRIORITY_HIGHEST + 1];
static	spnlck_t			sched_list_lock;

//CPU status
static	core_sched_info_t	cpu_infos[MAX_CPU_NUM];

//Thread table
static	array_t				thread_table;
static	spnlck_rw_t			thread_table_lock;

static int tmp_priority = PRIORITY_DISPATCH;

void core_pm_thread_init()
{
    //Initialize heap
    sched_heap = core_mm_heap_create(HEAP_MULITHREAD, SANDNIX_KERNEL_PAGE_SIZE);

    if(sched_heap == NULL) {
        PANIC(ENOMEM, "Failed to create schedule heap.");
    }

    //Initialize thread table
    core_rtl_array_init(&thread_table, MAX_THREAD_NUM, sched_heap);
    core_pm_spnlck_rw_init(&thread_table_lock);

    //Create thread 0
    core_pm_thread_core_init();
    pthread_obj_t p_thread_obj = thread_obj_0();

    if(p_thread_obj == NULL) {
        PANIC(ENOMEM, "Faile to create thread object for thread 0.");
    }

    core_rtl_array_set(&thread_table, 0, p_thread_obj);

    //Initialize schedule list
    core_rtl_memset(sched_lists, 0, sizeof(sched_lists));
    core_rtl_memset(cpu_infos, 0, sizeof(cpu_infos));
    core_pm_spnlck_init(&sched_list_lock);
}

void core_pm_thread_core_init()
{
    u32 cpu_index = hal_cpu_get_cpu_index();
    pcore_sched_info_t p_info = &cpu_infos[cpu_index];
    p_info->cpu_use_stat_l = 0;
    p_info->cpu_use_stat_h = 0;
    p_info->tick_count = 0;
    p_info->current_node = NULL;
    core_pm_spnlck_init(&(p_info->lock));

    return;
}

void core_pm_thread_core_release()
{
}

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

