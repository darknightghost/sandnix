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

#include "ssddt.h"
#include "ssddt_funcs.h"
#include "../../rtl/rtl.h"
#include "../../vfs/vfs.h"
#include "../../msg/msg.h"
#include "../../mm/mm.h"
#include "../../pm/pm.h"
#include "../../exceptions/exceptions.h"

void* ssddt_create_semaphore(va_list p_args)
{
	//Agruments
	u32 max_count;

	//Variables

	//Get args
}

k_status ssddt_acqr_semaphore(va_list p_args)
{
	//Agruments
	void* p_sem_obj;
	u32 timeout;

	//Variables

	//Get args
}

k_status ssddt_try_semaphore(va_list p_args)
{
	//Agruments
	void* p_sem_obj;

	//Variables

	//Get args
}

void ssddt_rls_semaphore(va_list p_args)
{
	//Agruments
	void* p_sem_obj;

	//Variables

	//Get args
}

void ssddt_destroy_semaphore(va_list p_args)
{
	//Agruments
	void* p_sem_obj;

	//Variables

	//Get args
}