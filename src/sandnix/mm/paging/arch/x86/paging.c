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

#include "paging.h"
#include "../../../../setup/setup.h"


void init_paging()
{
	return;
}

void* mm_virt_alloc(void* start_addr, size_t size, u32 options)
{
	return NULL;
}

void* mm_virt_free(void* start_addr, u32 options)
{
	return NULL;
}

u32 mm_pg_tbl_fork(u32 parent)
{
	return 0;
}

void mm_pg_tbl_free(u32 id)
{
	return 0;
}

void mm_pg_tbl_switch(u32 id)
{
	return 0;
}

void mm_get_info(pmem_info p_info)
{
	return 0;
}
