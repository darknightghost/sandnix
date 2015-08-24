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

#include "../vfs.h"
#include "../../rtl/rtl.h"

array_list_t		drivers_list;
array_list_t		devices_list;
hash_table_t		dev_mj_index;

static	u32			dev_mj_hash(char* name);
static	bool		dev_mj_name_cmp(pdev_mj_info_t p_dev1, pdev_mj_info_t p_dev2);

void om_init()
{
	rtl_array_list_init(&drivers_list, DEV_MJ_NUM_MAX, NULL);
	rtl_array_list_init(&devices_list, DEV_MJ_NUM_MAX * DEV_MN_NUM_MAX, NULL);
	rtl_hash_table_init(&dev_mj_index,
	                    0,
	                    0x0000FFFF / 2,
	                    (hash_func_t)dev_mj_hash,
	                    (compare_func_t)dev_mj_name_cmp,
	                    NULL);
}

u32 dev_mj_hash(char* name)
{
	UNREFERRED_PARAMETER(name);
	return 0;
}

bool dev_mj_name_cmp(pdev_mj_info_t p_dev1, pdev_mj_info_t p_dev2)
{
	if(rtl_strcmp(p_dev1->name, p_dev2->name) == 0) {
		return true;
	}

	return false;
}
