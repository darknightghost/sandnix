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

#include "ssddt.h"
#include "ssddt_funcs.h"
#include "../../../libs/driver/syscalls.h"
#include "../../rtl/rtl.h"

syscall_t	ssddt[SYSCALL_MAX];

void ssddt_init()
{
	rtl_memset(ssddt, 0, sizeof(ssddt));

	//Filesystem
	SYSTEM_CALL(SYS_OPEN, ssddt_open, ssddt);
	SYSTEM_CALL(SYS_FCHMOD, ssddt_fchmod, ssddt);
	SYSTEM_CALL(SYS_ACCESS, ssddt_access, ssddt);
	SYSTEM_CALL(SYS_CLOSE, ssddt_close, ssddt);
	SYSTEM_CALL(SYS_READ, ssddt_read, ssddt);
	SYSTEM_CALL(SYS_WRITE, ssddt_write, ssddt);
	SYSTEM_CALL(SYS_SEEK, ssddt_seek, ssddt);
	SYSTEM_CALL(SYS_STAT, ssddt_stat, ssddt);
	SYSTEM_CALL(SYS_UNLINK, ssddt_unlink, ssddt);
	SYSTEM_CALL(SYS_MKDIR, ssddt_mkdir, ssddt);
	SYSTEM_CALL(SYS_READDIR, ssddt_readdir, ssddt);

	/*
	//Mount
	SYSTEM_CALL(SYS_MOUNT, ssddt_mount, ssddt);
	SYSTEM_CALL(SYS_UMOUNT, ssddt_umount, ssddt);

	//Objects
	SYSTEM_CALL(SYS_CREATE_FILE_OBJ, ssddt_create_file_obj, ssddt);
	SYSTEM_CALL(SYS_CREATE_DRV_OBJ, ssddt_create_drv_obj, ssddt);
	SYSTEM_CALL(SYS_CREATE_DEV, ssddt_create_dev, ssddt);
	SYSTEM_CALL(SYS_REMOVE_DEV, ssddt_remove_dev, ssddt);
	SYSTEM_CALL(SYS_SET_DEV_FILENAME, ssddt_set_dev_filename, ssddt);
	SYSTEM_CALL(SYS_GET_MAJOR, ssddt_get_major, ssddt);
	SYSTEM_CALL(SYS_SYNC, ssddt_sync, ssddt);

	//Memory
	SYSTEM_CALL(SYS_VIRT_ALLOC, ssddt_virt_alloc, ssddt);
	SYSTEM_CALL(SYS_VIRT_FREE, ssddt_virt_free, ssddt);
	SYSTEM_CALL(SYS_MAP_PMO, ssddt_map_pmo, ssddt);
	SYSTEM_CALL(SYS_UNMAP_PMO, ssddt_unmap_pmo, ssddt);
	SYSTEM_CALL(SYS_MAP_RESERV_MEM, ssddt_map_reserv_mem, ssddt);
	SYSTEM_CALL(SYS_UMAP_RESERV_MEM, ssddt_umap_reserv_mem, ssddt);

	//Process
	SYSTEM_CALL(SYS_FORK, ssddt_fork, ssddt);
	SYSTEM_CALL(SYS_EXECVE, ssddt_execve, ssddt);
	SYSTEM_CALL(SYS_WAITPID, ssddt_waitpid, ssddt);
	SYSTEM_CALL(SYS_GET_PROC_ID, ssddt_get_proc_id, ssddt);
	SYSTEM_CALL(SYS_GET_UID, ssddt_get_uid, ssddt);
	SYSTEM_CALL(SYS_SET_UID, ssddt_set_uid, ssddt);
	SYSTEM_CALL(SYS_GET_GID, ssddt_get_gid, ssddt);
	SYSTEM_CALL(SYS_SET_GID, ssddt_set_gid, ssddt);
	SYSTEM_CALL(SYS_CHG_TO_USR, ssddt_chg_to_usr, ssddt);

	//Thread
	SYSTEM_CALL(SYS_SCHEDULE, ssddt_schedule, ssddt);
	SYSTEM_CALL(SYS_CREATE_THRD, ssddt_create_thrd, ssddt);
	SYSTEM_CALL(SYS_EXIT_THRD, ssddt_exit_thrd, ssddt);
	SYSTEM_CALL(SYS_SUSPEND, ssddt_suspend, ssddt);
	SYSTEM_CALL(SYS_JOIN, ssddt_join, ssddt);
	SYSTEM_CALL(SYS_RESUME, ssddt_resume, ssddt);
	SYSTEM_CALL(SYS_SLEEP, ssddt_sleep, ssddt);
	SYSTEM_CALL(SYS_GET_THRD_ID, ssddt_get_thrd_id, ssddt);
	SYSTEM_CALL(SYS_GET_ERRNO, ssddt_get_errno, ssddt);
	SYSTEM_CALL(SYS_SET_ERRNO, ssddt_set_errno, ssddt);

	//Mutex
	SYSTEM_CALL(SYS_CREATE_MUTEX, ssddt_create_mutex, ssddt);
	SYSTEM_CALL(SYS_ACQR_MUTEX, ssddt_acqr_mutex, ssddt);
	SYSTEM_CALL(SYS_TRY_MUTEX, ssddt_try_mutex, ssddt);
	SYSTEM_CALL(SYS_RLS_MUTEX, ssddt_rls_mutex, ssddt);

	//Semaphore
	SYSTEM_CALL(SYS_CREATE_SEMAPHORE, ssddt_create_semaphore, ssddt);
	SYSTEM_CALL(SYS_ACQR_SEMAPHORE, ssddt_acqr_semaphore, ssddt);
	SYSTEM_CALL(SYS_TRY_SEMAPHORE, ssddt_try_semaphore, ssddt);
	SYSTEM_CALL(SYS_RLS_SEMAPHORE, ssddt_rls_semaphore, ssddt);

	//Message
	SYSTEM_CALL(SYS_RECV_MSG, ssddt_recv_msg, ssddt);
	SYSTEM_CALL(SYS_COMPLETE_MSG, ssddt_complete_msg, ssddt);
	SYSTEM_CALL(SYS_FORWARD_MSG, ssddt_forward_msg, ssddt);
	SYSTEM_CALL(SYS_CANCEL_MSG, ssddt_cancel_msg, ssddt);

	//IO
	SYSTEM_CALL(SYS_READ_PORT, ssddt_read_port, ssddt);
	SYSTEM_CALL(SYS_WRITE_PORT, ssddt_write_port, ssddt);
	SYSTEM_CALL(SYS_GET_TICKCOUNT, ssddt_get_tickcount, ssddt);
	SYSTEM_CALL(SYS_GET_TICK, ssddt_get_tick, ssddt);
	SYSTEM_CALL(SYS_SET_INT_MSG, ssddt_set_int_msg, ssddt);
	SYSTEM_CALL(SYS_CLEAN_INT_MSG, ssddt_clean_int_msg, ssddt);

	//Others
	SYSTEM_CALL(SYS_KPRINT, ssddt_kprint, ssddt);
	*/
	return;
}
