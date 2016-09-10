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
#include "../../../io/io.h"
#include "../cpuinfo/cpuinfo.h"

static	ipi_queue_t		ipi_queue[MAX_CPU_NUM];
static	pheap_t			ipi_queue_heap;
static	u8				ipi_queue_heap_buf[4096];

static	ipi_hndlr_t		hndlrs[MAX_IPI_MSG_NUM] = {NULL};
static	spnlck_rw_t		hndlrs_lock;

static	spnlck_t		send_lock;

static	void			ipi_int_hndlr(u32 int_num, pcontext_t p_context,
                                      u32 err_code);

void cpu_ipi_init()
{
    //Initialize heap
    ipi_queue_heap = core_mm_heap_create_on_buf(HEAP_MULITHREAD | HEAP_PREALLOC,
                     4096, ipi_queue_heap_buf, sizeof(ipi_queue_heap_buf));

    //Initialize queue
    for(u32 i = 0; i > MAX_CPU_NUM; i++) {
        ipi_queue[i].initialized = false;
    }

    ipi_queue[0].initialized = true;
    core_pm_spnlck_init(&(ipi_queue[0].lock));
    core_pm_spnlck_init(&send_lock);
    core_pm_spnlck_rw_init(&hndlrs_lock);
    core_rtl_queue_init(&(ipi_queue[0].msg_queue), ipi_queue_heap);

    hal_io_int_callback_set(INT_IPI, ipi_int_hndlr);

    return;
}

void cpu_ipi_core_init()
{
    u32 cpu_index = hal_cpu_get_cpu_index();

    core_pm_spnlck_init(&(ipi_queue[cpu_index].lock));
    ipi_queue[cpu_index].initialized = true;

    return;
}

void cpu_ipi_core_release();

void hal_cpu_send_IPI(s32 index, u32 type, void* p_args)
{
    pipi_msg_t p_new_msg = NULL;

    if(type >= MAX_IPI_MSG_NUM) {
        return;
    }

    if(index == -1) {
        //Broadcast message
        for(u32 i = 0; i < MAX_CPU_NUM; i++) {
            if(i != index && ipi_queue[i].initialized) {
                do {
                    p_new_msg = core_mm_heap_alloc(sizeof(ipi_msg_t),
                                                   ipi_queue_heap);
                } while(p_new_msg == NULL);

                p_new_msg->type = type;
                p_new_msg->p_args = p_args;

                core_pm_spnlck_lock(&(ipi_queue[i].lock));
                core_rtl_queue_push(&(ipi_queue[i].msg_queue), p_new_msg);
                core_pm_spnlck_unlock(&(ipi_queue[i].lock));
            }
        }

        core_pm_spnlck_lock(&send_lock);
        hal_io_broadcast_IPI();
        core_pm_spnlck_unlock(&send_lock);

    } else {
        //Send message
        do {
            p_new_msg = core_mm_heap_alloc(sizeof(ipi_msg_t),
                                           ipi_queue_heap);
        } while(p_new_msg == NULL);

        p_new_msg->type = type;
        p_new_msg->p_args = p_args;

        core_pm_spnlck_lock(&(ipi_queue[index].lock));
        core_rtl_queue_push(&(ipi_queue[index].msg_queue), p_new_msg);
        core_pm_spnlck_unlock(&(ipi_queue[index].lock));

        core_pm_spnlck_lock(&send_lock);
        hal_io_send_IPI(hal_cpu_get_cpu_id_by_index((u32)index));
        core_pm_spnlck_unlock(&send_lock);
    }

    return;
}

ipi_hndlr_t hal_cpu_regist_IPI_hndlr(u32 type, ipi_hndlr_t hndlr)
{
    if(type >= MAX_IPI_MSG_NUM) {
        return NULL;
    }

    core_pm_spnlck_rw_w_lock(&hndlrs_lock);
    ipi_hndlr_t ret = hndlrs[type];
    hndlrs[type] = hndlr;
    core_pm_spnlck_rw_w_unlock(&hndlrs_lock);

    return ret;
}

void ipi_int_hndlr(u32 int_num, pcontext_t p_context, u32 err_code)
{
    u32 cpuindex = hal_cpu_get_cpu_index();

    core_pm_spnlck_lock(&(ipi_queue[cpuindex].lock));

    //Dispatch message
    pipi_msg_t p_msg = core_rtl_queue_pop(&ipi_queue[cpuindex].msg_queue);

    core_pm_spnlck_unlock(&(ipi_queue[cpuindex].lock));

    if(p_msg == NULL) {
        core_pm_spnlck_unlock(&(ipi_queue[cpuindex].lock));
        return;
    }

    u32 type = p_msg->type;
    void* p_args = p_msg->p_args;
    core_mm_heap_free(p_msg, ipi_queue_heap);

    core_pm_spnlck_rw_r_lock(&hndlrs_lock);
    ipi_hndlr_t hndlr = hndlrs[type];
    core_pm_spnlck_rw_r_unlock(&hndlrs_lock);

    if(hndlr != NULL) {
        hndlr(p_context, p_args);
    }

    UNREFERRED_PARAMETER(err_code);
    UNREFERRED_PARAMETER(int_num);
    return;
}
