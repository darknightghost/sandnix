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

#include "../../../hal/exception/exception.h"

#include "../../rtl/rtl.h"
#include "../../mm/mm.h"
#include "../../pm/pm.h"

#include "./ehpcorruption_except.h"
#include "../exception.h"

static	void		panic(pehpcorruption_except_t p_this);

pehpcorruption_except_t ehpcorruption_except(pheap_t corrpt_heap)
{
    pehpcorruption_except_t p_ret = (pehpcorruption_except_t)except_obj(
                                        sizeof(ehpcorruption_except_t),
                                        EHPCORRUPTION);

    if(p_ret != NULL) {
        p_ret->except.obj.class_id = CLASS_EXCEPT(EHPCORRUPTION);
        p_ret->heap = corrpt_heap;
        p_ret->except.panic = (except_panic_t)panic;
    }

    return p_ret;
}

void panic(pehpcorruption_except_t p_this)
{
    //Get call stack
    list_t bt_lst;
    hal_debug_backtrace(&bt_lst, p_this->except.p_context, p_except_obj_heap);

    pkstring_obj_t p_str_bt = kstring("", p_except_obj_heap);

    if(bt_lst != NULL) {
        plist_node_t p_node = bt_lst;

        do {
            pkstring_obj_t p_new_str = kstring_fmt("%k\n%p", p_except_obj_heap,
                                                   p_str_bt, p_node->p_item);
            DEC_REF(p_str_bt);
            p_str_bt = p_new_str;
            p_node = p_node->p_next;
        } while(p_node != bt_lst);
    }


    //Panic
    if(p_this->except.comment == NULL) {
        hal_exception_panic(p_this->except.file->buf,
                            p_this->except.line,
                            p_this->except.reason,
                            "Corrupted heap : %p.\n"
                            "Call stack:%k",
                            p_this->heap,
                            p_str_bt);

    } else {
        hal_exception_panic(p_this->except.file->buf,
                            p_this->except.line,
                            p_this->except.reason,
                            "Corrupted heap : %p.\n"
                            "%k\n"
                            "Call stact:%k",
                            p_this->except.comment,
                            p_this->heap,
                            p_str_bt);
    }

    return;
}
