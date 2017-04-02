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

#include "./thread_except_stat_obj.h"
#include "../pm/pm.h"
#include "../mm/mm.h"
#include "./exception.h"

static	void			destructor(pthread_except_stat_obj_t p_this);
static	int				compare(pthread_except_stat_obj_t p_this,
                                pthread_except_stat_obj_t p_1);
static	pkstring_obj_t				to_string(pthread_except_stat_obj_t p_this);
static	pthread_except_stat_obj_t	on_fork(pthread_except_stat_obj_t p_this,
        u32 thread_id,
        u32 process_id);

extern	pheap_t		p_except_heap;

#define	MODULE_NAME		core_exception

pthread_except_stat_obj_t thread_except_stat_obj(u32 thread_id, u32 process_id)
{
    //Create object
    pthread_except_stat_obj_t p_ret = (pthread_except_stat_obj_t)thread_ref_obj(
                                          thread_id,
                                          process_id,
                                          CLASS_THRAD_EXPECT_STAT,
                                          (thread_obj_fork_t)on_fork,
                                          (destructor_t)destructor,
                                          (compare_obj_t)compare,
                                          (to_string_t)to_string,
                                          p_except_heap,
                                          sizeof(thread_except_stat_obj_t));

    if(p_ret != NULL) {
        p_ret->errno = 0;
        core_rtl_stack_init(&(p_ret->hndlr_stack));
        core_pm_spnlck_rw_init(&(p_ret->lock));
    }

    return p_ret;
}

void destructor(pthread_except_stat_obj_t p_this)
{
    //Unwind all handlers
    while(!core_rtl_stack_empty(&(p_this->hndlr_stack))) {
        pexcept_hndlr_info_t p_info = (pexcept_hndlr_info_t)core_rtl_stack_pop(
                                          &(p_this->hndlr_stack), p_except_heap);
        p_info->hndlr(EXCEPT_REASON_UNWIND, NULL);
    }

    //Free memory
    core_mm_heap_free(p_this, p_this->parent.obj.heap);

    PRIVATE(release_stat_id)(p_this->parent.thread_id);

    return;
}

int compare(pthread_except_stat_obj_t p_this, pthread_except_stat_obj_t p_1)
{
    return (int)(p_this->parent.thread_id) - p_1->parent.thread_id;
}

pkstring_obj_t to_string(pthread_except_stat_obj_t p_this)
{
    pkstring_obj_t p_str = kstring_fmt(
                               "Thread id : %d\n"
                               "errno : %d\n",
                               p_this->parent.obj.heap,
                               p_this->parent.thread_id,
                               p_this->errno);
    return p_str;
}

pthread_except_stat_obj_t on_fork(pthread_except_stat_obj_t p_this, u32 thread_id, u32 process_id)
{
    UNREFERRED_PARAMETER(p_this);
    return thread_except_stat_obj(thread_id, process_id);
}
