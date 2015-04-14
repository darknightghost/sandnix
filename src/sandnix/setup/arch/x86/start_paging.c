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

#include "../../setup.h"

void start_paging()
{
	u32 i;
	u32 mapped_size;
	ppde p_pde;
	ppte p_pte;

	//Initialize PTE
	for(mapped_size = 0, p_pte = (p_pte)TMP_PAGE_TABLE_BASE;
		mapped_size < KERNEL_MAX_SIZE;
		mapped_size += 4 * 1024, p_pte++) {
		p_pte->present = PG_P;
		p_pte->read_write = PG_RW;
		p_pte->user_supervisor = PG_SUPERVISOR;
		p_pte->write_through = PG_WRITE_THROUGH;
		p_pte->cache_disabled = 0;
		p_pte->accessed = 0;
		p_pte->dirty = 0;
		p_pte->page_table_attr_index = 0;
		p_pte->global_page = 0;
		p_pte->avail = PG_NORMAL;
		p_pte->page_base_addr = mapped_size / (4 * 1024);
	}

	while(p_pte < tmp_page_table + TMP_PAGE_NUM / 4) {
		p_pte->present = PG_NP;
		p_pte->read_write = PG_RW;
		p_pte->user_supervisor = PG_SUPERVISOR;
		p_pte->write_through = PG_WRITE_THROUGH;
		p_pte->cache_disabled = 0;
		p_pte->accessed = 0;
		p_pte->dirty = 0;
		p_pte->page_table_attr_index = 0;
		p_pte->global_page = 0;
		p_pte->avail = PG_NORMAL;
		p_pte->page_base_addr = 0;
		p_pte++;
	}

	//Initialize PDE
	return;
}
