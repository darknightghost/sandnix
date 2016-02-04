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
	void* p_current;
	u32 boot_info_size;
	u32 tag_type;
	u32 tag_size;
	void* initrd_begin;
	void* initrd_end;
	pmultiboot_tag_module_t p_module_info;

	u32 test_flag = 0;

	//Boot info size
	p_current = p_boot_info;
	boot_info_size = *((u32*)p_current);
	p_current = (void*)((u32)p_current + sizeof(u32) * 2);

	initrd_begin = NULL;
	initrd_end = 0;

	//Read tags to find the position of initrd
	while((u32)p_current - (u32)p_boot_info > boot_info_size) {
		tag_type = *(u32*)p_current;
		p_current = (void*)((u32)p_current + sizeof(u32));
		tag_size =  *(u32*)p_current;

		if(tag_type == MULTIBOOT_TAG_TYPE_END) {
			break;
		}

		switch(tag_type) {
			case MULTIBOOT_TAG_TYPE_MODULE:
				p_current = (void*)((u32)p_current + sizeof(u32));
				p_current = (void*)((u32)p_current + (8 - (u32)p_current % 8));
				p_module_info = (pmultiboot_tag_module_t)p_current;
				initrd_begin = (void*)p_module_info->mod_start;
				initrd_end = (void*)p_module_info->mod_end;
				test_flag = 1;

				goto _TAG_END;

			default:
				p_current = (void*)((u32)p_current + tag_size - sizeof(u32));
		}

		//Make the pointer 8-bytes aligned
		if((u32)p_current % 8 != 0) {
			p_current = (void*)((u32)p_current + (8 - (u32)p_current % 8));
		}
	}

_TAG_END:

	__asm__ __volatile__(
	    "movl	%0,%%eax\n"
	    "movl	%1,%%ebx\n"
	    "movl	%2,%%ecx\n"
	    "_loop:\n"
	    "jmp	_loop\n"
	    ::"m"(initrd_begin), "m"(initrd_end), "m"(test_flag));

	UNREFERRED_PARAMETER(offset);
	UNREFERRED_PARAMETER(p_boot_info);
	UNREFERRED_PARAMETER(p_base);
	UNREFERRED_PARAMETER(p_num);
	return;
}

