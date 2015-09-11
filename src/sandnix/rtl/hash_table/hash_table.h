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

#ifndef	HASH_TABLE_H_INCLUDE
#define	HASH_TABLE_H_INCLUDE

#include "../rtl.h"

typedef	u32(*hash_func_t)(void*);
typedef	bool(*compare_func_t)(void*, void*);
typedef	struct {
	u32				base;
	u32				max;
	array_list_t	table;
	hash_func_t		hash_func;
	compare_func_t	compare_func;
} hash_table_t, *phash_table_t;

typedef	struct	{
	void*	index;
	void*	p_item;
} hash_table_index_node_t, *phash_table_index_node_t;

k_status	rtl_hash_table_init(phash_table_t p_table,
                                u32 base,
                                u32 max,
                                hash_func_t hash_func,
                                compare_func_t compare_func,
                                void* heap);
k_status	rtl_hash_table_set(phash_table_t p_table,
                               void* index,
                               void* item,
                               void* heap);
void*		rtl_hash_table_get(phash_table_t p_table,
                               void* index);
void		rtl_hash_table_remove(phash_table_t p_table,
                                  void* index,
                                  void* heap);
void		rtl_hash_table_destroy(phash_table_t p_table,
                                   void* heap);

#endif	//!	HASH_TABLE_H_INCLUDE
