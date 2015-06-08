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

#ifdef	X86

	#include "../../common/arch/x86/types.h"
	#include "../../common/arch/x86/kernel_image.h"

#endif	//X86

#include "../debug/debug.h"
#include "parameters/parameters.h"
#include "../exceptions/exceptions.h"
#include "../io/io.h"
#include "../mm/mm.h"

void kernel_main()
{
	char* a;
	dbg_cls();
	io_set_crrnt_int_level(INT_LEVEL_DISPATCH);

	dbg_print("%s", "Sandnix 0.0.1 kernel loaded.\n");

	get_kernel_param();
	init_io();

	excpt_init();

	mm_init();

	a = mm_virt_alloc((void*)0xD0000000, 4096,
	                  MEM_RESERVE | MEM_COMMIT,
	                  PAGE_WRITEABLE);
	dbg_print("a = %p\n", a);

	if(a != NULL) {
		rtl_strcpy_s(a, 4096, "Hello world!\n");
		dbg_print("%s\n", a);
	}

	mm_virt_free(a, 4096, MEM_RELEASE);
	dbg_print("%s", "Freed.\n");
	__asm__ __volatile__(
	    ".global _a\n\t"
	    "_a:\n\t"
	    "movl	%0,%%eax\n\t"
	    "movl	$0,(%%eax)\n\t"
	    ::"m"(a));
	dbg_print("%s", "Alloc Tested.\n");

	while(1);

	io_dispatch_int();

	return;
}
