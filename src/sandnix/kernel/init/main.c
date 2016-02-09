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

#include "../../../common/common.h"
#include "../../boot/multiboot2.h"
#include "init.h"
#include "../io/io.h"
#include "../debug/debug.h"
#include "../exceptions/exceptions.h"
#include "../rtl/rtl.h"


u8		init_stack[KERNEL_STACK_SIZE];

void kernel_main(u32 magic, pmultiboot_tag_t p_load_info)
{
	dbg_init();
	dbg_kprint("%s loading...\n", VER_STR);

	io_init();

	while(1);

	UNREFERRED_PARAMETER(magic);
	UNREFERRED_PARAMETER(p_load_info);
	return;
}
