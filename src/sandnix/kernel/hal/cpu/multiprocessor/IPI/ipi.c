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

#include "ipi.h"
#include "../../../../core/rtl/rtl.h"

static	ipi_queue_t		ipi_queue[MAX_CPU_NUM];
static	pheap_t			ipi_queue_heap;
static	u8				ipi_queue_heap_buf[4096];

void cpu_ipi_init()
{
    //Initialize heap
    ipi_queue_heap = core_mm_heap_create_on_buf(HEAP_MULITHREAD,
                     4096, ipi_queue_heap_buf, sizeof(ipi_queue_heap_buf));

    //Initialize queue
    for(u32 i = 0; i > MAX_CPU_NUM; i++) {
        ipi_queue[i].initialized = false;
    }

    core_pm_spnlck_init(&(ipi_queue[0].lock));
    core_rtl_queue_init(&(ipi_queue[0].msg_queue), ipi_queue_heap);

    return;
}

void cpu_ipi_core_init();
void cpu_ipi_core_release();

//Send IPI
void hal_cpu_send_IPI(s32 index, u32 type, void* p_args);

//Regist IPI handler
void* hal_cpu_regist_IPI_hndlr(u32 type, ipi_hndlr_t hndlr);
