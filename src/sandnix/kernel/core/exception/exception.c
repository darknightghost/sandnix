/*
    Copyright 2016,王思远 <darknightghost.cn@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "../rtl/rtl.h"
#include "../mm/mm.h"
#include "../pm/pm.h"

#include "../../hal/cpu/cpu.h"
#include "../../hal/exception/exception.h"

#include "../kconsole/kconsole.h"

#include "exception.h"

#define	MODULE_NAME		core_exception

pheap_t		p_except_heap = NULL;

static	bool		initialized = false;
static	spnlck_rw_t	globl_list_lock;
static	list_t		globl_except_hndlr_list;
static	u8			except_heap_buf[4096 * 4];

static	void		call_thread_hndlrs(pexcept_obj_t except);
static	void		call_globl_hndlrs(pexcept_obj_t except);
static	pthread_except_stat_obj_t	thread_ref_call_back(u32 thread_id, u32 process_id);
static	spnlck_rw_t	except_info_tbl_lck;
static	array_t		except_info_tbl;

void core_exception_init()
{
    core_kconsole_print_info("\nInitializaing exception module...\n");

    //Initialize heap
    p_except_heap = core_mm_heap_create_on_buf(HEAP_PREALLOC | HEAP_MULITHREAD,
                    SANDNIX_KERNEL_PAGE_SIZE,
                    except_heap_buf,
                    sizeof(except_heap_buf));

    if(p_except_heap == NULL) {
        PANIC(ENOMEM, "Failed to create heap for exception objects.\n");
    }

    //Initialize list
    core_rtl_list_init(&globl_except_hndlr_list);
    core_rtl_array_init(&except_info_tbl, MAX_PROCESS_NUM, p_except_heap);
    core_pm_spnlck_rw_init(&globl_list_lock);
    core_pm_spnlck_rw_init(&except_info_tbl_lck);
    initialized = true;

    //Create except infomation struct for thread 0
    pthread_except_stat_obj_t p_stat_0 = thread_except_stat_obj(0, 0);
    core_pm_spnlck_rw_w_lock(&except_info_tbl_lck);

    if(core_rtl_array_set(&except_info_tbl, 0, p_stat_0) == NULL) {
        PANIC(ENOMEM,
              "Failed to create thread exception status object for thread 0.");
    }

    core_pm_spnlck_rw_w_unlock(&except_info_tbl_lck);
    return;
}

void PRIVATE(release_stat_id)(u32 id)
{
    core_pm_spnlck_rw_w_lock(&except_info_tbl_lck);

    core_rtl_array_set(&except_info_tbl, id, NULL);

    core_pm_spnlck_rw_w_unlock(&except_info_tbl_lck);

    return;
}

void core_exception_regist_thrd_ref()
{
    core_pm_reg_thread_ref_obj((thread_ref_call_back_t)thread_ref_call_back);
    return;
}

void core_exception_set_errno(kstatus_t status)
{
    //Get thread info
    u32 priority;
    core_pm_spnlck_rw_r_lock(&except_info_tbl_lck, &priority);
    pthread_except_stat_obj_t p_thread_stat = (pthread_except_stat_obj_t)core_rtl_array_get(
                &except_info_tbl, core_pm_get_currnt_thread_id());

    if(p_thread_stat == NULL) {
        core_pm_spnlck_rw_r_unlock(&except_info_tbl_lck, priority);
        PANIC(EINVAL, "Failed to get exception status object of curent thread.");
    }

    INC_REF(p_thread_stat);
    core_pm_spnlck_rw_r_unlock(&except_info_tbl_lck, priority);

    //Set errno
    core_pm_spnlck_rw_w_lock(&(p_thread_stat->lock));
    p_thread_stat->errno = status;
    core_pm_spnlck_rw_w_unlock(&(p_thread_stat->lock));
    DEC_REF(p_thread_stat);

    return;
}

kstatus_t core_exception_get_errno()
{
    //Get thread info
    u32 priority;
    core_pm_spnlck_rw_r_lock(&except_info_tbl_lck, &priority);
    pthread_except_stat_obj_t p_thread_stat = (pthread_except_stat_obj_t)core_rtl_array_get(
                &except_info_tbl, core_pm_get_currnt_thread_id());

    if(p_thread_stat == NULL) {
        core_pm_spnlck_rw_r_unlock(&except_info_tbl_lck, priority);
        PANIC(EINVAL, "Failed to get exception status object of curent thread.");
    }

    INC_REF(p_thread_stat);
    core_pm_spnlck_rw_r_unlock(&except_info_tbl_lck, priority);

    //Get errno
    core_pm_spnlck_rw_r_lock(&(p_thread_stat->lock), &priority);
    u32 ret = p_thread_stat->errno;
    core_pm_spnlck_rw_r_unlock(&(p_thread_stat->lock), priority);
    DEC_REF(p_thread_stat);

    return ret;
}

void core_exception_raise(pexcept_obj_t except)
{
    if(!initialized) {
        except->panic(except);
        return;
    }

    u32 priority;
    core_pm_spnlck_rw_r_lock(&except_info_tbl_lck, &priority);
    pthread_except_stat_obj_t p_ret = (pthread_except_stat_obj_t)core_rtl_array_get(
                                          &except_info_tbl,
                                          core_pm_get_currnt_thread_id());
    p_ret->errno = except->reason;
    core_pm_spnlck_rw_r_unlock(&except_info_tbl_lck, priority);

    //Call thread exception handler stack
    call_thread_hndlrs(except);

    //Call global exception handlers
    call_globl_hndlrs(except);

    //No handler found.
    if(hal_cpu_is_kernel_context(except->p_context)) {
        //IP is in kernel memory panic
        except->panic(except);

    } else {
        //IP is in user memory, kill process
        NOT_SUPPORT;
    }
}

plist_node_t core_exception_add_hndlr(kstatus_t reason, except_hndlr_t hndlr)
{
    plist_node_t ret = NULL;

    //Allocate memory
    pexcept_hndlr_info_t p_info = core_mm_heap_alloc(
                                      sizeof(except_hndlr_info_t),
                                      p_except_heap);

    if(p_info != NULL) {
        //Fill info
        p_info->reason = reason;
        p_info->hndlr = hndlr;

        //Add to list
        core_pm_spnlck_rw_w_lock(&globl_list_lock);
        ret = core_rtl_list_insert_after(NULL, &globl_except_hndlr_list,
                                         p_info, p_except_heap);
        core_pm_spnlck_rw_w_unlock(&globl_list_lock);

        if(ret == NULL) {
            core_mm_heap_free(p_info, p_except_heap);
        }
    }

    return ret;
}

void core_exception_remove_hndlr(plist_node_t pos)
{
    pexcept_hndlr_info_t p_info = (pexcept_hndlr_info_t)(pos->p_item);
    core_pm_spnlck_rw_w_lock(&globl_list_lock);
    core_rtl_list_remove(pos, &globl_except_hndlr_list, p_except_heap);
    core_pm_spnlck_rw_w_unlock(&globl_list_lock);
    core_mm_heap_free(p_info, p_except_heap);

    return;
}

//Push exception hndlr of current thread
except_ret_stat_t core_exception_do_push_hndlr(pexcept_hndlr_info_t p_hndlr_info,
        pcontext_t p_context)
{
    //Copy context
    core_rtl_memcpy(&(p_hndlr_info->context), p_context, sizeof(context_t));

    //Get handler stack
    u32 priority;
    core_pm_spnlck_rw_r_lock(&except_info_tbl_lck, &priority);
    pthread_except_stat_obj_t p_thread_stat = (pthread_except_stat_obj_t)core_rtl_array_get(
                &except_info_tbl, core_pm_get_currnt_thread_id());

    if(p_thread_stat == NULL) {
        core_pm_spnlck_rw_r_unlock(&except_info_tbl_lck, priority);
        PANIC(EINVAL, "Failed to get exception status object of curent thread.");
    }

    INC_REF(p_thread_stat);
    core_pm_spnlck_rw_r_unlock(&except_info_tbl_lck, priority);

    //Push handler
    core_pm_spnlck_rw_w_lock(&(p_thread_stat->lock));

    if(core_rtl_stack_push(&(p_thread_stat->hndlr_stack),
                           p_hndlr_info,
                           p_except_heap) == NULL) {
        core_pm_spnlck_rw_w_unlock(&(p_thread_stat->lock));
        DEC_REF(p_thread_stat);
        PANIC(ENOMEM, "Failed to push thread handler for curent thread.");
    }

    core_pm_spnlck_rw_w_unlock(&(p_thread_stat->lock));
    DEC_REF(p_thread_stat);

    return EXCEPT_RET_PUSH;
}

//Pop exception hndlr of current thread
pexcept_hndlr_info_t core_exception_pop_hndlr()
{
    //Get handler stack
    u32 priority;
    core_pm_spnlck_rw_r_lock(&except_info_tbl_lck, &priority);
    pthread_except_stat_obj_t p_thread_stat = (pthread_except_stat_obj_t)core_rtl_array_get(
                &except_info_tbl, core_pm_get_currnt_thread_id());

    if(p_thread_stat == NULL) {
        core_pm_spnlck_rw_r_unlock(&except_info_tbl_lck, priority);
        PANIC(EINVAL, "Failed to get exception status object of curent thread.");
    }

    INC_REF(p_thread_stat);
    core_pm_spnlck_rw_r_unlock(&except_info_tbl_lck, priority);

    //Push handler
    core_pm_spnlck_rw_w_lock(&(p_thread_stat->lock));

    pexcept_hndlr_info_t p_ret = core_rtl_stack_pop(
                                     &(p_thread_stat->hndlr_stack),
                                     p_except_heap);
    core_pm_spnlck_rw_w_unlock(&(p_thread_stat->lock));
    DEC_REF(p_thread_stat);

    return p_ret;
}

void call_thread_hndlrs(pexcept_obj_t except)
{
    context_t context;

    //Get handler stack
    u32 priority;
    core_pm_spnlck_rw_r_lock(&except_info_tbl_lck, &priority);
    pthread_except_stat_obj_t p_thread_stat = (pthread_except_stat_obj_t)core_rtl_array_get(
                &except_info_tbl, core_pm_get_currnt_thread_id());
    INC_REF(p_thread_stat);
    core_pm_spnlck_rw_r_unlock(&except_info_tbl_lck, priority);

    core_pm_spnlck_rw_r_lock(&(p_thread_stat->lock), &priority);
    stack_t p_stack = p_thread_stat->hndlr_stack;

    if(!core_rtl_list_empty(&(p_thread_stat->hndlr_stack))) {
        //Looking for handlers
        plist_node_t p_node = p_stack->p_prev;

        do {
            pexcept_hndlr_info_t p_info = p_node->p_item;

            if(p_info->reason == except->reason) {
                except_stat_t status = p_info->hndlr(EXCEPT_REASON_EXCEPT,
                                                     except);

                switch(status) {
                    case EXCEPT_STATUS_CONTINUE_EXEC:
                        //Continue execution
                        core_pm_spnlck_rw_r_unlock(&(p_thread_stat->lock), priority);
                        core_rtl_memcpy(&context, except->p_context, sizeof(context_t));
                        DEC_REF(except);
                        hal_cpu_context_load(&context);
                        break;

                    case EXCEPT_STATUS_UNWIND:
                        //Unwind handlers
                        core_pm_spnlck_rw_r_unlock(&(p_thread_stat->lock), priority);
                        core_pm_spnlck_rw_w_lock(&(p_thread_stat->lock));

                        while(p_stack->p_prev != p_node) {
                            pexcept_hndlr_info_t p_tmp_info = core_rtl_stack_pop(
                                                                  &(p_thread_stat->hndlr_stack),
                                                                  p_thread_stat->parent.obj.heap);
                            p_tmp_info->hndlr(EXCEPT_REASON_UNWIND, NULL);
                        }

                        core_pm_spnlck_rw_w_unlock(&(p_thread_stat->lock));
                        DEC_REF(p_thread_stat);
                        core_rtl_memcpy(&context, &(p_info->context),
                                        sizeof(context_t));
                        DEC_REF(except);
                        hal_cpu_context_load(&context);
                        break;

                    case EXCEPT_STATUS_CONTINUE_SEARCH:
                        //Search for next handler
                        p_node = p_node->p_prev;
                        continue;

                    case EXCEPT_STATUS_PANIC:
                        //Panic
                        core_pm_spnlck_rw_r_unlock(&(p_thread_stat->lock), priority);
                        except->panic(except);
                        break;

                    default:
                        //Unknow return value, panic
                        core_pm_spnlck_rw_r_unlock(&(p_thread_stat->lock), priority);
                        PANIC(EIRETVAL,
                              "Illegal return value 0x%.8X in exception handler %p.",
                              status,
                              p_info->hndlr);
                        break;
                }
            }

            p_node = p_node->p_prev;
        } while(p_node != p_stack->p_prev);

    }

    core_pm_spnlck_rw_r_unlock(&(p_thread_stat->lock), priority);

    return;
}

void call_globl_hndlrs(pexcept_obj_t except)
{
    context_t context;

    if(core_rtl_list_empty(&globl_except_hndlr_list)) {
        //No handler found. Panic.
        except->panic(except);

    } else {
        //Looking for handlers
        u32 priority;
        core_pm_spnlck_rw_r_lock(&globl_list_lock, &priority);
        plist_node_t p_node;
        p_node = globl_except_hndlr_list->p_prev;

        do {
            pexcept_hndlr_info_t p_info = p_node->p_item;

            if(p_info->reason == except->reason) {
                except_stat_t status = p_info->hndlr(EXCEPT_REASON_EXCEPT,
                                                     except);

                switch(status) {
                    case EXCEPT_STATUS_CONTINUE_EXEC:
                        //Continue execution
                        core_pm_spnlck_rw_r_unlock(&globl_list_lock, priority);
                        core_rtl_memcpy(&context, except->p_context, sizeof(context_t));
                        DEC_REF(except);
                        hal_cpu_context_load(&context);
                        break;

                    case EXCEPT_STATUS_CONTINUE_SEARCH:
                        //Search for next handler
                        p_node = p_node->p_prev;
                        continue;

                    case EXCEPT_STATUS_PANIC:
                        //Panic
                        core_pm_spnlck_rw_r_unlock(&globl_list_lock, priority);
                        except->panic(except);
                        break;

                    default:
                        //Unknow return value, panic
                        core_pm_spnlck_rw_r_unlock(&globl_list_lock, priority);
                        PANIC(EIRETVAL,
                              "Illegal return value 0x%.8X in exception handler %p.",
                              status,
                              p_info->hndlr);
                        break;
                }
            }

            p_node = p_node->p_prev;
        } while(p_node != globl_except_hndlr_list->p_prev);

        core_pm_spnlck_rw_r_unlock(&globl_list_lock, priority);
    }

    return;
}

pthread_except_stat_obj_t thread_ref_call_back(u32 thread_id, u32 process_id)
{
    if(thread_id == 0) {
        u32 priority;
        core_pm_spnlck_rw_r_lock(&except_info_tbl_lck, &priority);
        pthread_except_stat_obj_t p_ret = (pthread_except_stat_obj_t)core_rtl_array_get(
                                              &except_info_tbl, 0);
        core_pm_spnlck_rw_r_unlock(&except_info_tbl_lck, priority);

        return p_ret;

    } else {
        pthread_except_stat_obj_t p_stat = NULL;

        while(p_stat == NULL) {
            p_stat = thread_except_stat_obj(thread_id, process_id);
        }

        core_pm_spnlck_rw_w_lock(&except_info_tbl_lck);

        if(core_rtl_array_set(&except_info_tbl, thread_id, p_stat) == NULL) {
            PANIC(ENOMEM,
                  "Failed to create thread exception status object for thread 0.");
        }

        core_pm_spnlck_rw_w_unlock(&except_info_tbl_lck);

        return p_stat;
    }
}
