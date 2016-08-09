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

#pragma once

#include "../../../../../../common/common.h"
#include "../container.h"
#include "../../../mm/mm.h"

typedef	struct	_array_blk {
    size_t		count;
    size_t		ref;
    void*		p_datas;
} array_blk_t, *parray_blk_t;


typedef	struct	_array {
    size_t			size;
    size_t			scale;
    pheap_t			heap;
    parray_blk_t	p_blks;
} array_t, *parray_t;

void core_rtl_array_init(
    parray_t p_array,		//数组地址
    u32 num,				//大小上限
    pheap_t heap);			//堆

//获得元素
void* core_rtl_array_get(
    parray_t p_array,		//数组地址
    u32 index);				//下标

//设置n元素值,设成NULL即remove
void* core_rtl_array_set(
    parray_t p_array,		//数组地址
    u32 index,				//下标
    void* value);			//值

//检测该元素是否被赋值
bool core_rtl_array_used(
    parray_t p_array,		//数组地址
    u32 index);				//下标

//获得当前元素个数
u32 core_rtl_array_size{
    parray_t p_array};		//数组地址

//获得最大索引值
u32 core_rtl_array_get_current_max_index(
    parray_t p_array);		//数组地址

//获得一个空闲的索引
u32 core_rtl_array_get_free_index(
    parray_t p_array);		//数组地址

//获得空闲索引个数
u32 core_rtl_array_get_free_index_num(
    parray_t p_array);		//数组地址

//销毁
void core_rtl_array_destroy(
    parray_t p_array,			//数组地址
    item_destroyer_t destroier,	//销毁元素回调
    void* arg);					//回调函数参数
