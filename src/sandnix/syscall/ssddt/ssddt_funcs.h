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

#include "../../../common/common.h"
#include "../../rtl/rtl.h"

#ifndef	SSDDT_FUNCS_H_INCLUDE
#define	SSDDT_FUNCS_H_INCLUDE

//Filesystem
u32			ssddt_open(va_list p_args);
k_status	ssddt_fchmod(va_list p_args);
k_status	ssddt_access(va_list p_args);
void		ssddt_close(va_list p_args);
size_t		ssddt_read(va_list p_args);
size_t		ssddt_write(va_list p_args);
k_status	ssddt_seek(va_list p_args);
k_status	ssddt_stat(va_list p_args);
k_status	ssddt_unlink(va_list p_args);
k_status	ssddt_mkdir(va_list p_args);
size_t		ssddt_readdir(va_list p_args);

//Mount
k_status	ssddt_mount(va_list p_args);
k_status	ssddt_umount(va_list p_args);

//Objects
u32			ssddt_create_file_obj();
u32			ssddt_create_drv_obj(va_list p_args);
u32			ssddt_create_dev(va_list p_args);
u32			ssddt_remove_dev(va_list p_args);
u32			ssddt_set_dev_filename(va_list p_args);
u32			ssddt_get_major(va_list p_args);
k_status	ssddt_sync(va_list p_args);

//Memory
void*		ssddt_virt_alloc(va_list p_args);
void		ssddt_virt_free(va_list p_args);
void*		ssddt_map_pmo(va_list p_args);
void		ssddt_unmap_pmo(va_list p_args);
void*		ssddt_map_reserv_mem(va_list p_args);
void		ssddt_umap_reserv_mem(va_list p_args);

//Process
int			ssddt_fork();
void		ssddt_execve(va_list p_args);
u32			ssddt_waitpid(va_list p_args);
u32			ssddt_get_proc_id();
u32			ssddt_get_uid();
k_status	ssddt_set_uid(va_list p_args);
u32			ssddt_get_gid();
k_status	ssddt_set_gid(va_list p_args);
void		ssddt_chg_to_usr();

//Thread
void		ssddt_schedule();
u32			ssddt_create_thrd(va_list p_args);
void		ssddt_exit_thrd(va_list p_args);
void		ssddt_suspend(va_list p_args);
u32			ssddt_join(va_list p_args);
void		ssddt_resume(va_list p_args);
void		ssddt_sleep(va_list p_args);
u32			ssddt_get_thrd_id();
u32			ssddt_get_errno();
void		ssddt_set_errno(va_list p_args);

//Mutex
void*		ssddt_create_mutex();
k_status	ssddt_acqr_mutex(va_list p_args);
k_status	ssddt_try_mutex(va_list p_args);
void		ssddt_rls_mutex(va_list p_args);

//Semaphore
void*		ssddt_create_semaphore(va_list p_args);
k_status	ssddt_acqr_semaphore(va_list p_args);
k_status	ssddt_try_semaphore(va_list p_args);
void		ssddt_rls_semaphore(va_list p_args);

//Message
void*		ssddt_recv_msg(va_list p_args);
k_status	ssddt_complete_msg(va_list p_args);
k_status	ssddt_forward_msg(va_list p_args);
void		ssddt_cancel_msg(va_list p_args);

//IO
void		ssddt_read_port(va_list p_args);
void		ssddt_write_port(va_list p_args);
void		ssddt_read_port_datas(va_list p_args);
void		ssddt_write_port_datas(va_list p_args);
void		ssddt_get_tickcount(va_list p_args);
u32			ssddt_get_tick();
k_status	ssddt_set_int_msg(va_list p_args);
void		ssddt_clean_int_msg(va_list p_args);
void		ssddt_kprint(va_list p_args);

#endif	//!	SSDDT_FUNCS_H_INCLUDE
