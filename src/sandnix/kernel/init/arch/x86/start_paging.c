/*
	  Copyright 2016,暗夜幽灵 <darknightghost.cn@gmail.com>

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

#include "../../../../../common/common.h"
#include "../../../../boot/multiboot2.h"
#include "../../init.h"
#include "../../../mm/mm.h"

static	void	get_needed_pages(u32 offset, void* p_boot_info,
                                 void** p_base, u32* p_num);

void* start_paging(u32 offset, u32 magic, void* p_boot_info)
{
	void* base;
	u32 page_num;

	get_needed_pages(offset, p_boot_info, &base, &page_num);

	while(1);

	UNREFERRED_PARAMETER(magic);
	UNREFERRED_PARAMETER(p_boot_info);
	UNREFERRED_PARAMETER(offset);
	return NULL;
}

void get_needed_pages(u32 offset, void* p_boot_info,
                      void** p_base, u32* p_num)
{
	//void* p_current;
	u32 boot_info_size = *((u32*)p_boot_info);

	__asm__ __volatile__(
	    "movl	%0,%%eax\n"
	    "movl	%1,%%ebx\n"
	    "_loop:\n"
	    "jmp	_loop\n"
	    ::"m"(p_boot_info), "m"(boot_info_size));

	UNREFERRED_PARAMETER(offset);
	UNREFERRED_PARAMETER(p_boot_info);
	UNREFERRED_PARAMETER(p_base);
	UNREFERRED_PARAMETER(p_num);
	return;
}

