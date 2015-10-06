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

void* ssddt_recv_msg(va_list p_args)
{
	//Agruments
	pmsg_t buf;
	size_t buf_size;
	size_t* p_msg_len;
	bool if_block;

	//Variables

	//Get args
}

k_status ssddt_complete_msg(va_list p_args)
{
	//Agruments
	void* p_msg;
	k_status status;

	//Variables

	//Get args
}

k_status ssddt_forward_msg(va_list p_args)
{
	//Agruments
	void* p_msg;
	u32 dev_num;

	//Variables

	//Get args
}

void ssddt_cancel_msg(va_list p_args)
{
	//Agruments
	void* p_msg;

	//Variables

	//Get args
}
