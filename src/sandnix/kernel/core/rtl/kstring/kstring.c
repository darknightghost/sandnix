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

#include "kstring.h"
#include "../../mm/mm.h"
#include "../string/string.h"

//obj
static	void			destructer(pkstring_obj_t p_this);
static	int				compare(pkstring_obj_t p_this, pkstring_obj_t p_str);
static	pkstring_obj_t	to_string(pkstring_obj_t p_this);

//kstring
static	size_t			kstring_len(pkstring_obj_t p_this);
static	pkstring_obj_t	copy(pkstring_obj_t p_this, pheap_t heap);
static	pkstring_obj_t	substr(pkstring_obj_t p_this, u32 begin, u32 end);
static	pkstring_obj_t	append(pkstring_obj_t p_this, pkstring_obj_t p_str);
static	pkstring_obj_t	upper(pkstring_obj_t p_this);
static	pkstring_obj_t	lower(pkstring_obj_t p_this);
static	int				search(pkstring_obj_t p_this, pkstring_obj_t p_substr);

pkstring_obj_t kstring(char* str, pheap_t heap)
{
    //Allocate memory
    pkstring_obj_t p_ret = (pkstring_obj_t)obj(CLASS_KSTRING,
                           (destructor_t)destructer,
                           (compare_obj_t)compare, (to_string_t)to_string,
                           heap, sizeof(kstring_obj_t));

    if(p_ret == NULL) {
        return NULL;
    }

    size_t len = core_rtl_strlen(str) + 1;
    p_ret->buf = core_mm_heap_alloc(len, heap);

    if(p_ret == NULL) {
        core_mm_heap_free(p_ret, heap);
        return NULL;
    }

    core_rtl_strncpy(p_ret->buf, str, len);

    //Initialize methods
    p_ret->len = kstring_len;
    p_ret->copy = copy;
    p_ret->substr = substr;
    p_ret->append = append;
    p_ret->upper = upper;
    p_ret->lower = lower;
    p_ret->search = search;

    return p_ret;
}

void destructer(pkstring_obj_t p_this)
{
    core_mm_heap_free(p_this->buf, p_this->obj.heap);
    core_mm_heap_free(p_this, p_this->obj.heap);
    return;
}

int compare(pkstring_obj_t p_this, pkstring_obj_t p_str)
{
    return core_rtl_strcmp(p_this->buf, p_str->buf);
}

pkstring_obj_t to_string(pkstring_obj_t p_this)
{
    INC_REF(p_this);
    return p_this;
}

size_t kstring_len(pkstring_obj_t p_this)
{
    return core_rtl_strlen(p_this->buf);
}

pkstring_obj_t copy(pkstring_obj_t p_this, pheap_t heap)
{
    return kstring(p_this->buf, heap);
}

pkstring_obj_t substr(pkstring_obj_t p_this, u32 begin, u32 end)
{
    char* p_buf = core_mm_heap_alloc(end - begin + 2, p_this->obj.heap);

    if(p_buf == NULL) {
        return NULL;
    }

    core_rtl_strncpy(p_buf, p_this->buf + begin, end - begin + 1);
    pkstring_obj_t p_ret = kstring(p_buf, p_this->obj.heap);
    core_mm_heap_free(p_buf, p_this->obj.heap);
    return p_ret;
}

pkstring_obj_t append(pkstring_obj_t p_this, pkstring_obj_t p_str)
{
    size_t sz = p_this->len(p_this) + p_str->len(p_this) + 1;
    char* buf = core_mm_heap_alloc(sz, p_this->obj.heap);

    if(buf == NULL) {
        return NULL;
    }

    core_rtl_strncpy(buf, p_this->buf, sz);
    core_rtl_strncat(buf, p_str->buf, sz);

    pkstring_obj_t p_ret = kstring(buf, p_this->obj.heap);
    core_mm_heap_free(buf, p_this->obj.heap);
    return p_ret;
}

pkstring_obj_t upper(pkstring_obj_t p_this)
{
    pkstring_obj_t p_ret = kstring(p_this->buf, p_this->obj.heap);

    if(p_ret != NULL) {
        for(char* p = p_ret->buf;
            *p != '\0';
            p++) {
            if(*p >= 'a' && *p <= 'z') {
                *p += 'A' - 'a';
            }
        }

    }

    return p_ret;
}

pkstring_obj_t lower(pkstring_obj_t p_this)
{
    pkstring_obj_t p_ret = kstring(p_this->buf, p_this->obj.heap);

    if(p_ret != NULL) {
        for(char* p = p_ret->buf;
            *p != '\0';
            p++) {
            if(*p >= 'A' && *p <= 'Z') {
                *p += 'a' - 'A';
            }
        }

    }

    return p_ret;
}

int search(pkstring_obj_t p_this, pkstring_obj_t p_substr)
{
    char* pos = core_rtl_strstr(p_this->buf, p_substr->buf);

    if(pos == NULL) {
        return -1;

    } else {
        return pos - p_this->buf;
    }
}
