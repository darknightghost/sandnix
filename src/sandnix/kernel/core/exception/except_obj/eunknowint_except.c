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

#include "./eunknowint_except.h"
#include "../exception.h"


static pkstring_obj_t		to_string(peunknowint_except_t p_this);
static void					panic(peunknowint_except_t p_this);

peunknowint_except_t	eunknowint_except(u32 int_num, u32 error_code)
{
    peunknowint_except_t p_ret = (peunknowint_except_t)except_obj(
                                     sizeof(eunknowint_except_t),
                                     EUNKNOWINT);

    if(p_ret != NULL) {
        p_ret->except.obj.class_id = CLASS_EXCEPT(EUNKNOWINT);
        p_ret->int_num = int_num;
        p_ret->err_code = error_code;
        p_ret->except.panic = (except_panic_t)panic;
        p_ret->except.obj.to_string = (to_string_t)to_string;
    }

    return p_ret;
}

pkstring_obj_t to_string(peunknowint_except_t p_this)
{
    pkstring_obj_t p_ret = kstring_fmt(
                               "%s\n"
                               "Interrupt number = %u, Error code = %u",
                               p_except_obj_heap,
                               hal_exception_get_err_name(p_this->except.reason),
                               p_this->int_num,
                               p_this->err_code);
    return p_ret;
}

void panic(peunknowint_except_t p_this)
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
                            "Interrupt Number = %u, Error code = %u.\n"
                            "Call stack:%k",
                            p_this->int_num,
                            p_this->err_code,
                            p_str_bt);

    } else {
        hal_exception_panic(p_this->except.file->buf,
                            p_this->except.line,
                            p_this->except.reason,
                            "Interrupt Number = %u, Error code = %u.\n"
                            "%k\nCall stact:%k",
                            p_this->int_num,
                            p_this->err_code,
                            p_this->except.comment,
                            p_str_bt);
    }

    return;
}
