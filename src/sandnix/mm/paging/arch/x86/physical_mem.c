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

#include "physical_mem.h"
#include "../../../../../common/arch/x86/kernel_image.h"

static	u32		phy_mem_info[1024 * 1024];

void setup_e820()
{
	u32 num;
	u32 i;
	pe820_table p_table;

	//Get e820 addr
	num = **(u32**)KERNEL_MEM_INFO;
	p_table = (pe820_table)(*(u32**)KERNEL_MEM_INFO + 1);

	//Anlyse e820
	for(i = 0; i < 1024 * 1024; i++) {
		if(PHYSICAL_PAGE_SIZE * i < 0xA0000) {
			phy_mem_info[i] = 0x02;
		}

		if(PHYSICAL_PAGE_SIZE * i > p_table->base_addr + p_table->len) {

		}
	}
}
