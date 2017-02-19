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

#include "./process_obj.h"
#include "../../rtl/rtl.h"
#include "../../mm/mm.h"
#include "../../exception/exception.h"

/*
//Heap
static	pheap_t				proc_obj_heap = NULL;

//Obj methods
static	pkstring_obj_t		to_string(pprocess_obj_t p_this);
static	int					compare(pprocess_obj_t p_this, pprocess_obj_t p_obj2);
static	void				destructor(pprocess_obj_t p_this);

//Process object methods
pprocess_obj_t		fork(pprocess_obj_t p_this);
void				add_ref(pprocess_obj_t p_this, pproc_ref_obj_t p_ref_obj);
void				die(pprocess_obj_t p_this);


pprocess_obj_t process_obj_0()
{
    //Initialize heap
    proc_obj_heap = core_mm_heap_create(HEAP_MULITHREAD, 4096);

    if(proc_obj_heap == NULL) {
        penomem_except_t p_except = enomem_except();
        RAISE(p_except, "Failed to create heap for process objects.");
        return NULL;
    }

    //Create process object 0
    pprocess_obj_t p_ret = (pprocess_obj_t)obj(
                               CLASS_PROCESS_OBJ,
                               (destructor_t)destructor,
                               (compare_obj_t)compare,
                               (to_string_t)to_string,
                               proc_obj_heap,
                               sizeof(process_obj_t));

    if(p_ret == NULL) {
        penomem_except_t p_except = enomem_except();
        RAISE(p_except, "Failed to create process object.");
        return NULL;
    }

    //Member variables
    p_ret->process_id = 0;
    p_ret->parent_id = 0;
    p_ret->exit_code = 0;

    //Authority
    p_ret->ruid = 0;
    p_ret->euid = 0;
    p_ret->suid = 0;
    p_ret->rgid = 0;
    p_ret->egid = 0;
    p_ret->sgid = 0;
    core_rtl_list_init(&(p_ret->groups));

    //Environment
    core_rtl_map_init(&(p_ret->env_vars),
                      (item_compare_t)core_rtl_strcmp,
                      proc_obj_heap);

    //Threads
    p_ret->alive_thread_num = 0;
}
*/
