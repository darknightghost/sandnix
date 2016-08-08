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

#include "kparam.h"
#include "../init/init.h"
#include "../mmu/mmu.h"
#include "../../core/rtl/rtl.h"
#include "../exception/exception.h"
#include "../early_print/early_print.h"

static	map_t		krnl_param_map;

static	void		jump_space(char** p_p);
static	char*		get_word(char** p_p);

void hal_kparam_init()
{
    char* p_cmdline;
    char* p;
    char* p_value;
    char* p_key;

    hal_early_print_printf("\nAnalysing kernel parameters...\n");

    p_cmdline = hal_mmu_add_early_paging_addr(hal_init_get_kernel_cmdline());
    core_rtl_map_init(&krnl_param_map, (item_compare_t)core_rtl_strcmp, NULL);

    //Analyse parameters
    for(p = p_cmdline;
        *p != '\0';
        p++) {
        jump_space(&p);
        //Get key
        p_key = get_word(&p);
        jump_space(&p);

        //Get value
        if(*p == '=') {
            p++;
            jump_space(&p);
            p_value = get_word(&p);

        } else {
            p_value = "";
        }

        hal_early_print_printf("%s = \"%s\".\n", p_key, p_value);

        if(core_rtl_map_set(&krnl_param_map, p_key, p_value) == NULL) {
            hal_exception_panic(ENOMEM, "Failed to allocate memory for kernel parameter.");
        }
    }

    return;
}

kstatus_t hal_kparam_get_value(char* key, char* buf, size_t size)
{
    char* p_ret;

    p_ret = core_rtl_map_get(&krnl_param_map, key);

    if(p_ret == NULL) {
        return ENOENT;
    }

    core_rtl_strncpy(buf, p_ret, size);

    if(size < core_rtl_strlen(p_ret) + 1) {
        return EOVERFLOW;

    } else {
        return ESUCCESS;
    }
}

void jump_space(char** p_p)
{
    for(;**p_p == ' ' ||**p_p == '\t' || **p_p == '\n'; (*p_p)++);

    return;
}

char* get_word(char** p_p)
{
    char* p_start;
    bool quote_flag;
    bool slash_flag;
    char* p_ret;
    size_t len;

    p_start = *p_p;
    quote_flag = false;
    slash_flag = false;

    while(true) {

        if(**p_p == '\0') {
            break;

        } else if(slash_flag) {
            (*p_p)++;
            continue;

        } else if(**p_p == '\"') {
            quote_flag = !quote_flag;

        } else if((**p_p == '=' || **p_p == ' ' || **p_p == '\t') && !quote_flag) {
            break;

        } else {
            (*p_p)++;
        }

        slash_flag = false;
    }

    len = (address_t)(*p_p) - (address_t)p_start + 1;
    p_ret = core_mm_heap_alloc(len, NULL);

    if(p_ret == NULL) {
        hal_exception_panic(ENOMEM, "Failed to allocate memory for kernel parameter.");
    }

    core_rtl_strncpy(p_ret, p_start, len - 1);

    return p_ret;
}
