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

#ifndef	FUNCS_H_INCLUDE
#define	FUNCS_H_INCLUDE

//Fileysytem
u32			open(const char *pathname, u32 flags, u32 mode);
k_status	fchmod(u32 fd, u32 mode);
k_status	access(const char *pathname, u32 mode);
void		close(u32 fd);
size_t		read(u32 fd, void *buf, size_t count);
size_t		write(u32 fd, const void *buf, size_t count);
void		seek(u32 fd, u64 offset, u32 whence, u64* p_after_pos);
k_status	stat(const char *path, struct stat *buf);
k_status	unlink(const char *pathname);
k_status	mkdir(const char *pathname, u32 mode);

//Mount
k_status	mount(const char *source, const char *target,
                  const char *filesystemtype, u32 mountflags,
                  const char *args);
k_status	umount(const char *target);

//Objects
u32			create_file_obj();
u32			create_drv_obj(char* drv_name);
u32			create_dev(char* name, u32 major, u32 gid,
                       u32 block_size, u32 parent);
u32			remove_dev(u32 dev_num);
u32			set_dev_filename(u32 dev_num, char* file_name);
u32			get_major(char* num);
k_status	sync(u32 dev_num);

//Memory
void*		virt_alloc(void* addr, size_t size, u32 options, u32 attr);
void		virt_free(void* start_addr, size_t size, u32 options);
void*		map_pmo(void* address, ppmo_t p_pmo);
void		unmap_pmo(void* address, ppmo_t p_pmo);
void*		map_reserv_mem(void* virt_addr, void* phy_addr);
void		unmap_reserv_mem(void* virt_addr);

//Process
int			fork();
void		execve(const char *filename, char *const argv[],
                   char *const envp[]);
u32			waitpid(u32 pid, u32 *status, u32 options);
u32			get_proc_id();
u32			get_uid();
int			set_uid(u32 uid);
u32			get_gid();
int			set_gid(u32 gid);
void		chg_to_usr();

//Thread
void		schedule();
u32			create_thrd(void (start_addr*)(void*), u32 priority,
                        bool is_ready, void* p_args);
void		exit_thrd(u32 exit_code);
void		suspend(u32 thread_id);
u32			join(u32 thread_id);
void		resume(u32 thread_id);
void		sleep(u32 ms);
u32			get_thrd_id();
u32			get_errno();
void		set_errno(u32 errno);

//Mutex
void*		create_mutex();
k_status	acqr_mutex(void* p_mutex_obj, u32 timeout);
k_status	try_mutex(void* p_mutex_obj);
void		rls_mutex(void* p_mutex_obj);

//Semaphore
void*		create_semaphore();
k_status	acqr_semaphore(void* p_sem_obj, u32 timeout, u32 max_count);
k_status	try_semaphore(void* p_sem_obj);
void		rls_semaphore(void* p_sem_obj);

//Message
void*		recv_msg(pmsg_t buf, size_t buf_size,
                     size_t* p_msg_len, bool if_block);
k_status	complete_msg(void* p_msg, k_status status);
k_status	forward_msg(void* p_msg, u32 dev_num);
void		cancel_msg(void* p_msg);

//IO
void		read_port(void* buf, u32 bits, u32 len);
void		write_port(void* buf, u32 bits, u32 len);
void		get_tickcount(u64* tick_count);
u32			get_tick();
k_status	set_int_msg(u32 int_num);
void		clean_int_msg(u32 int_num);

//Others
u32			kprint(char* fmt, ...);

#endif	//!	FUNCS_H_INCLUDE
