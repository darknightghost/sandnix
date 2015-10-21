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

#include "../exceptions/exceptions.h"
#include "../vfs/vfs.h"
#include "../pm/pm.h"
#include "../rtl/rtl.h"
#include "../debug/debug.h"
#include "../io/io.h"
#include "stdio.h"

static	char	line_buf[1024];

void ls(char* path);
void cat(char* path);
void echo(char* str);

void demo_main(u32 thread_id, void* p_null)
{
	char* p;

	stdio_init();

	while(1) {
		dbg_print("#>");
		read_line(line_buf);

		for(p = line_buf;
		    *p != '\0';
		    p++) {
			if(*p == ' ') {
				*p = '\0';
				p++;
				break;
			}
		}

		if(rtl_strcmp(line_buf, "ls") == 0) {
			ls(p);

		} else if(rtl_strcmp(line_buf, "echo") == 0) {
			echo(p);

		} else {

			dbg_print("Unknow command!\n");
		}
	}

	UNREFERRED_PARAMETER(thread_id);
	UNREFERRED_PARAMETER(p_null);
}

void ls(char* path)
{

	u32 fd;
	ppmo_t p_pmo;
	pmsg_readdir_info_t p_info;
	pmsg_readdir_data_t p_data;
	pdirent_t p_dir;
	u8* p;

	fd = vfs_open(path, O_RDONLY | O_DIRECTORY, 0);

	if(!OPERATE_SUCCESS) {
		dbg_print("No such directory!\n");
		return;
	}

	p_pmo = mm_pmo_create(4096);

	if(p_pmo == NULL) {
		dbg_print("Readdir Failed\n");
		return;
	}

	p_info = mm_pmo_map(NULL, p_pmo, false);

	if(p_info == NULL) {
		dbg_print("Readdir Failed\n");

		return;
	}

	p_info->count = 200;

	vfs_readdir(fd, p_pmo);

	if(!OPERATE_SUCCESS) {
		dbg_print("Readdir Failed\n");
		return;
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

void echo(char* str)
{
	dbg_print("%s\n", str);
	return;
}
