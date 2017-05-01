/*
    Copyright 2017,王思远 <darknightghost.cn@gmail.com>

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

#include "./file_obj.h"
#include "./msg/msg.h"
#include "../mm/mm.h"
#include "../pm/pm.h"
#include "../rtl/rtl.h"
#include "../exception/exception.h"
#include "../../hal/mmu/mmu.h"

#define	MODULE_NAME		core_vfs

//Heap
static	pheap_t			file_obj_heap = NULL;

//File object table
static	array_t			file_obj_table;
static	mutex_t			file_obj_table_lock;

static	void			destructor(pfile_obj_t p_this);
static	int				compare(pfile_obj_t p_this, pfile_obj_t p_obj);
static	pkstring_obj_t	to_string(pfile_obj_t p_this);

static	kstatus_t		send(pfile_obj_t p_this, pmsg_obj_t p_msg,
                             mstatus_t* result);
static	kstatus_t		recv(pfile_obj_t p_this, pmsg_obj_t* p_ret,
                             s32 millisec_timeout);
static	void			destroy(pfile_obj_t p_this);
static	void			lock_obj(pfile_obj_t p_this);
static	void			unlock_obj(pfile_obj_t p_this);

void PRIVATE(file_obj_init)()
{
    //Create heap
    file_obj_heap = core_mm_heap_create(HEAP_MULITHREAD, SANDNIX_KERNEL_PAGE_SIZE);

    if(file_obj_heap == NULL) {
        penomem_except_t p_except = enomem_except();
        RAISE(p_except, "Failed to create heap for file objects.");
        return;
    }

    //Initialze file object table
    core_rtl_array_init(&file_obj_table, MAX_FILEOBJ_NUM, file_obj_heap);
    core_pm_mutex_init(&file_obj_table_lock);

    return;
}

pfile_obj_t file_obj(u32 class_id, u32 inode, size_t size)
{
    //Check size
    if(size < sizeof(file_obj_t)) {
        return NULL;
    }

    //Allocate id
    core_pm_mutex_acquire(&file_obj_table_lock, -1);
    u32 new_id;

    if(core_rtl_array_get_free_index(
           &file_obj_table,
           &new_id) == false) {
        core_pm_mutex_release(&file_obj_table_lock);
        return NULL;
    }

    //Create object
    pfile_obj_t p_ret = (pfile_obj_t)obj(
                            class_id,
                            (destructor_t)destructor,
                            (compare_obj_t)compare,
                            (to_string_t)to_string,
                            file_obj_heap,
                            size);

    if(p_ret == NULL) {
        core_pm_mutex_release(&file_obj_table_lock);
        return NULL;
    }

    //Attributes
    p_ret->file_obj_id = new_id;
    core_rtl_array_set(&file_obj_table, new_id, p_ret);
    core_pm_mutex_release(&file_obj_table_lock);
    core_pm_mutex_init(&(p_ret->lock));
    p_ret->alive = true;
    p_ret->inode = inode;
    p_ret->msg_queue = msg_queue(file_obj_heap);

    //Methods
    p_ret->send = send;
    p_ret->recv = recv;
    p_ret->destroy = destroy;
    p_ret->lock_obj = lock_obj;
    p_ret->unlock_obj = unlock_obj;

    return p_ret;
}

pfile_obj_t core_vfs_get_file_obj_by_id(u32 id)
{
    core_pm_mutex_acquire(&file_obj_table_lock, -1);
    pfile_obj_t p_ret = core_rtl_array_get(&file_obj_table, id);

    if(p_ret != NULL) {
        INC_REF(p_ret);
    }

    core_pm_mutex_release(&file_obj_table_lock);

    return p_ret;
}

void destructor(pfile_obj_t p_this)
{
    //Release id
    core_pm_mutex_acquire(&file_obj_table_lock, -1);
    core_rtl_array_set(&file_obj_table, p_this->file_obj_id, NULL);
    core_pm_mutex_release(&file_obj_table_lock);

    //Free memory
    core_mm_heap_free(p_this, p_this->obj.heap);
    return;
}

int compare(pfile_obj_t p_this, pfile_obj_t p_obj)
{
    if((address_t)p_this > (address_t)p_obj) {
        return 1;

    } else if((address_t)p_this == (address_t)p_obj) {
        return 0;

    } else {
        return -1;
    }
}

pkstring_obj_t to_string(pfile_obj_t p_this)
{
    return kstring("File object.", p_this->obj.heap);
}

kstatus_t send(pfile_obj_t p_this, pmsg_obj_t p_msg, mstatus_t* result)
{
    return p_this->msg_queue->send(
               p_this->msg_queue,
               p_msg,
               result);
}

kstatus_t recv(pfile_obj_t p_this, pmsg_obj_t* p_ret, s32 millisec_timeout)
{
    return p_this->msg_queue->recv(
               p_this->msg_queue,
               p_ret,
               millisec_timeout);
}

void destroy(pfile_obj_t p_this)
{
    p_this->lock_obj(p_this);

    if(p_this->alive) {
        p_this->alive = false;
        p_this->msg_queue->destroy(p_this->msg_queue);
    }

    p_this->unlock_obj(p_this);

    return;
}

void lock_obj(pfile_obj_t p_this)
{
    core_pm_mutex_acquire(&(p_this->lock), -1);
    return;
}

void unlock_obj(pfile_obj_t p_this)
{
    core_pm_mutex_release(&(p_this->lock));
    return;
}

