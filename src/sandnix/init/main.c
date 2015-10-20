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

#include "../../common/common.h"
#include "../debug/debug.h"
#include "parameters/parameters.h"
#include "../exceptions/exceptions.h"
#include "../io/io.h"
#include "../mm/mm.h"
#include "../pm/pm.h"
#include "../rtl/rtl.h"
#include "../msg/msg.h"
#include "../syscall/syscall.h"


void test();

void kernel_main()
{
	dbg_init();

	dbg_cls();

	dbg_print("%s", "Sandnix 0.0.1 kernel loaded.\n");

	io_set_crrnt_int_level(INT_LEVEL_DISPATCH);

	//Initialize
	get_kernel_param();
	io_init();
	excpt_init();
	mm_init();
	pm_init();
	//io_int_msg_init();

	//Create daemon threads
	io_enable_interrupt();
	dbg_print("Creating interrupt dispatcher thread...\n");
	pm_create_thrd(io_dispatch_int, true, false, INT_LEVEL_EXCEPTION, NULL);

	//Initialize
	msg_init();
	vfs_init();

	io_int_msg_init();

	syscall_init();

	io_set_crrnt_int_level(INT_LEVEL_USR_HIGHEST);

	//Create driver_init process

	test();

	//IDLE
	io_set_crrnt_int_level(INT_LEVEL_IDLE);

	while(1);

	return;
}

void fs_test()
{
	u32 fd;
	ppmo_t p_pmo;
	pmsg_readdir_info_t p_info;
	pmsg_readdir_data_t p_data;
	pdirent_t p_dir;
	u8* p;

	fd = vfs_open("/drivers", O_RDONLY | O_DIRECTORY, 0);
	p_pmo = mm_pmo_create(4096);

	if(p_pmo == NULL) {
		dbg_print("Failed\n");

		while(1);
	}

	p_info = mm_pmo_map(NULL, p_pmo, false);

	if(p_info == NULL) {
		dbg_print("Failed\n");

		while(1);
	}

	p_info->count = 200;

	vfs_readdir(fd, p_pmo);

	if(!OPERATE_SUCCESS) {
		dbg_print("Failed\n");

		while(1);
	}

	p_data = (pmsg_readdir_data_t)p_info;
	p = &(p_data->data);

	while(p - & (p_data->data) < p_data->count) {
		p_dir = (pdirent_t)(p);
		dbg_print("%s\n", &(p_dir->d_name));
		p += p_dir->d_reclen + sizeof(dirent_t) - 1;
	}

	mm_pmo_unmap(p_info, p_pmo);
	mm_pmo_free(p_pmo);
	vfs_close(fd);

	return;
}

void test()
{

	dbg_print("<test>\n");

	fs_test();
	fs_test();

	dbg_print("</test>\n");

	while(1);
}
