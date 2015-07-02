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

#include "../../pm.h"

u32			pm_fork();
void		pm_exit(u32 exit_code);
u32			pm_wait(u32 thread_id);
void		pm_exec(char* cmd_line);
u32 pm_switch_process(u32 process_id)
{
	return 0;
}
u32			pm_get_pdt_id(u32 process_id);

u32 pm_get_proc_id(u32 thread_id)
{
	return 0;
}

u32			pm_get_proc_uid(u32 process_id);
bool		pm_set_proc_uid(u32 process_id, u32 uid);

