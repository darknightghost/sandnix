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

#include "../../../../../../common/common.h"
#include "../../../../init/init.h"
#include "../../../../rtl/rtl.h"
#include "../../../../debug/debug.h"
#include "../../../../exceptions/exceptions.h"
#include "../../../mm.h"
#include "../../../phymem/phymem.h"

void*	mm_mgr_page_addr;
void*	mm_init_pt_addr;
void*	kernel_address_offset;
u32		init_page_num;

void paging_init_arch(plist_t p_kernel_free_page_list,
                      plist_t p_kernel_using_page_list,
                      void* heap)
{
	*p_kernel_free_page_list = NULL;
	*p_kernel_using_page_list = NULL;

	dbg_kprint("%P\n%P\n%P\n%P\n", mm_mgr_page_addr, mm_init_pt_addr, kernel_address_offset, init_page_num);

	while(1);

	UNREFERRED_PARAMETER(heap);
}
