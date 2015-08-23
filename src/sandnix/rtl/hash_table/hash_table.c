/*
	Copyright 2015,暗夜幽灵 <darknightghost.cn@gmail.com>

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

#include "hash_table.h"
#include "../../mm/mm.h"
#include "../../pm/pm.h"
#include "../../exceptions/exceptions.h"


k_status rtl_hash_table_init(phash_table_t p_table,
                             u32 base,
                             u32 max,
                             hash_func_t hash_func,
                             void* heap)
{
	p_table->base = base;
	p_table->max = max;
	p_table->hash_func = hash_func;
	rtl_array_list_init(&(p_table->table), max - base + 1, heap);
	return ESUCCESS;
}

k_status rtl_hash_table_set(phash_table_t p_table,
                            void* index,
                            void* p_item,
                            void* heap)
{
	u32 hash;

	hash = p_table->hash_func(index);

	return rtl_array_list_set(&(p_table->table),
	                          hash - p_table->base,
	                          p_item,
	                          heap);
}

void* rtl_hash_table_get(phash_table_t p_table,
                         void* index)
{
	u32 hash;

	hash = p_table->hash_func(index);

	return rtl_array_list_get(&(p_table->table), hash - p_table->base);
}

void rtl_hash_table_remove(phash_table_t p_table,
                           void* index,
                           void* heap)
{
	u32 hash;

	hash = p_table->hash_func(index);

	rtl_array_list_release(&(p_table->table), hash - p_table->base, heap);

	return;
}

void rtl_hash_table_destroy(phash_table_t p_table,
                            item_destroyer_callback callback,
                            void* heap)
{
	rtl_array_list_destroy(&(p_table->table),
	                       callback,
	                       heap);
	return;
}
