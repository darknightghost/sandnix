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

#include "../../../../common/common.h"
#include "../../rtl/rtl.h"
#include "../../debug/debug.h"
#include "phymem.h"

list_t		phymem_list;
list_t		phymem_allocatable_list;

static	void	print_phymem();

void phymem_init()
{
	dbg_kprint("Testing physical memory...\n");

	#ifdef	X86
	phymem_init_x86();
	#endif	//!	X86

	print_phymem();
	return;
}

void print_phymem()
{
}
