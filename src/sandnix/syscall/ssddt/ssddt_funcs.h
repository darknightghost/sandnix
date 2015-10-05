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

#ifndef	SSDDT_FUNCS_H_INCLUDE
#define	SSDDT_FUNCS_H_INCLUDE

//Filesystem
u32			ssddt_open(void* p_args);
k_status	ssddt_fchmod(void* p_args);
k_status	ssddt_access(void* p_args);
void		ssddt_close(void* p_args);
size_t		ssddt_read(void* p_args);
size_t		ssddt_write(void* p_args);
k_status	ssddt_seek(void* p_args);
k_status	ssddt_stat(void* p_args);
k_status	ssddt_unlink(void* p_args);
k_status	ssddt_mkdir(void* p_args);
size_t		ssddt_readdir(void* p_args);

//Mount
k_status	ssddt_mount(void* p_args);
k_status	ssddt_umount(void* p_args);

//Objects
u32			ssddt_create_file_obj();
u32			ssddt_create_drv_obj(void* p_args);
u32			ssddt_create_dev(void* p_args);
u32			ssddt_remove_dev(void* p_args);
u32			ssddt_set_dev_filename(void* p_args);
u32			ssddt_get_major(void* p_args);
k_status	ssddt_sync(void* p_args);

//Memory
void*		ssddt_virt_alloc(void* p_args);
void		ssddt_virt_free(void* p_args);
void*		ssddt_map_pmo(void* p_args);
void		ssddt_unmap_pmo(void* p_args);
void*		ssddt_map_reserv_mem(void* p_args);
void		ssddt_umap_reserv_mem(void* p_args);

//Process
int			ssddt_fork();
void		ssddt_execve(void* p_args);
u32			ssddt_waitpid(void* p_args);
u32			ssddt_get_proc_id();
u32			ssddt_get_uid();
k_status	ssddt_set_uid(void* p_args);
u32			ssddt_get_gid();
k_status	ssddt_set_gid(void* p_args);
void		ssddt_chg_to_usr();

//Thread
void		ssddt_schedule();
u32			ssddt_create_thrd(void* p_args);
void		ssddt_exit_thrd(void* p_args);
void		ssddt_suspend(void* p_args);
u32			ssddt_join(void* p_args);
void		ssddt_resume(void* p_args);
void		ssddt_sleep(void* p_args);
u32			ssddt_get_thrd_id();
u32			ssddt_get_errno();
void		ssddt_set_errno(void* p_args);

//Mutex
void*		ssddt_create_mutex(void* p_args);
k_status	ssddt_acqr_mutex(void* p_args);
k_status	ssddt_try_mutex(void* p_args);
void		ssddt_rls_mutex(void* p_args);

//Semaphore
void*		ssddt_create_semaphore(void* p_args);
k_status	ssddt_acqr_semaphore(void* p_args);
k_status	ssddt_try_semaphore(void* p_args);
void		ssddt_rls_semaphore(void* p_args);

//Message
void*		ssddt_recv_msg(void* p_args);
k_status	ssddt_complete_msg(void* p_args);
k_status	ssddt_forward_msg(void* p_args);
void		ssddt_cancel_msg(void* p_args);

//IO
void		ssddt_read_port(void* p_args);
void		ssddt_write_port(void* p_args);
void		ssddt_get_tickcount(void* p_args);
u32			ssddt_get_tick();
k_status	ssddt_set_int_msg(void* p_args);
void		ssddt_clean_int_msg(void* p_args);

//Others
u32			ssddt_kprint(void* p_args);

#endif	//!	SSDDT_FUNCS_H_INCLUDE
