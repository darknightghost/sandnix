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

#include "../../../../../common/common.h"
#include "./thread_obj_defs.h"

//Create new thread object
pthread_obj_t		thread_obj(
    u32 thread_id,					//Thread id
    u32 process_id,					//Process id
    size_t kernel_stack_size,		//Size of new kernel stack
    u32 priority);					//Priority of new thread

//Create thread object for thread 0.
pthread_obj_t		thread_obj_0();
