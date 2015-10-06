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

#include "../../syscalls.h"

.section	.text

//Fileysytem
.global		open
.global		fchmod
.global		access
.global		close
.global		read
.global		write
.global		seek
.global		stat
.global		unlink
.global		mkdir
.global		readdir

//Mount
.global		mount
.global		umount

//Objects
.global		create_file_obj
.global		create_drv_obj
.global		create_dev
.global		remove_dev
.global		set_dev_filename
.global		get_major
.global		sync

//Memory
.global		virt_alloc
.global		virt_free
.global		map_pmo
.global		unmap_pmo
.global		map_reserv_mem
.global		unmap_reserv_mem

//Process
.global		fork
.global		execve
.global		waitpid
.global		get_proc_id
.global		get_uid
.global		set_uid
.global		get_gid
.global		set_gid
.global		chg_to_usr

//Thread
.global		schedule
.global		create_thrd
.global		exit_thrd
.global		suspend
.global		join
.global		resume
.global		sleep
.global		get_thrd_id
.global		get_errno
.global		set_errno

//Mutex
.global		create_mutex
.global		acqr_mutex
.global		try_mutex
.global		rls_mutex

//Semaphore
.global		create_semaphore
.global		acqr_semaphore
.global		try_semaphore
.global		rls_semaphore

//Message
.global		recv_msg
.global		complete_msg
.global		forward_msg
.global		cancel_msg

//IO
.global		read_port
.global		write_port
.global		get_tickcount
.global		get_tick
.global		set_int_msg
.global		clean_int_msg

//Others
.global		kprint

.code32

//Fileysytem
open:
			movl	$SYS_OPEN,%eax
			jmp		do_sys_call
fchmod:
			movl	$SYS_FCHMOD,%eax
			jmp		do_sys_call

access:
			movl	$SYS_ACCESS,%eax
			jmp		do_sys_call
close:
			movl	$SYS_CLOSE,%eax
			jmp		do_sys_call
read:
			movl	$SYS_READ,%eax
			jmp		do_sys_call
write:
			movl	$SYS_WRITE,%eax
			jmp		do_sys_call
seek:
			movl	$SYS_SEEK,%eax
			jmp		do_sys_call
stat:
			movl	$SYS_STAT,%eax
			jmp		do_sys_call
unlink:
			movl	$SYS_UNLINK,%eax
			jmp		do_sys_call
mkdir:
			movl	$SYS_MKDIR,%eax
			jmp		do_sys_call
readdir:
			movl	$SYS_READDIR,%eax
			jmp		do_sys_call

//Mount
mount:
			movl	$SYS_MOUNT,%eax
			jmp		do_sys_call
umount:
			movl	$SYS_UMOUNT,%eax
			jmp		do_sys_call

//Objects
create_file_obj:
			movl	$SYS_CREATE_FLIE_OBJ,%eax
			jmp		do_sys_call
create_drv_obj:
			movl	$SYS_CREATE_DRV_OBJ,%eax
			jmp		do_sys_call
create_dev:
			movl	$SYS_CREATE_DEV,%eax
			jmp		do_sys_call
remove_dev:
			movl	$SYS_REMOVE_DEV,%eax
			jmp		do_sys_call
set_dev_filename:
			movl	$SYS_SET_DEV_FILENAME,%eax
			jmp		do_sys_call
get_major:
			movl	$SYS_GET_MAJOR,%eax
			jmp		do_sys_call
sync:
			movl	$SYS_SYNC,%eax
			jmp		do_sys_call

//Memory
virt_alloc:
			movl	$SYS_VIRT_ALLOC,%eax
			jmp		do_sys_call
virt_free:
			movl	$SYS_VIRT_FREE,%eax
			jmp		do_sys_call
map_pmo:
			movl	$SYS_MAP_PMO,%eax
			jmp		do_sys_call
unmap_pmo:
			movl	$SYS_UNMAP_PMO,%eax
			jmp		do_sys_call
map_reserv_mem:
			movl	$SYS_MAP_RESERV_MEM,%eax
			jmp		do_sys_call
unmap_reserv_mem:
			movl	$SYS_UNMAP_RESERV_MEM,%eax
			jmp		do_sys_call

//Process
fork:
			movl	$SYS_FORK,%eax
			jmp		do_sys_call
execve:
			movl	$SYS_EXECVE,%eax
			jmp		do_sys_call
waitpid:
			movl	$SYS_WAITPID,%eax
			jmp		do_sys_call
get_proc_id:
			movl	$SYS_GET_PROC_ID,%eax
			jmp		do_sys_call
get_uid:
			movl	$SYS_GET_UID,%eax
			jmp		do_sys_call
set_uid:
			movl	$SYS_SET_UID,%eax
			jmp		do_sys_call
get_gid:
			movl	$SYS_GET_GID,%eax
			jmp		do_sys_call
set_gid:
			movl	$SYS_SET_GID,%eax
			jmp		do_sys_call
chg_to_usr:
			movl	$SYS_CHG_TO_USR,%eax
			jmp		do_sys_call

//Thread
schedule:
			movl	$SYS_SCHEDULE,%eax
			jmp		do_sys_call
create_thrd:
			movl	$SYS_CREATE_THRD,%eax
			jmp		do_sys_call
exit_thrd:
			movl	$SYS_EXIT_THRD,%eax
			jmp		do_sys_call
suspend:
			movl	$SYS_SUSPEND,%eax
			jmp		do_sys_call
join:
			movl	$SYS_JOIN,%eax
			jmp		do_sys_call
resume:
			movl	$SYS_RESUME,%eax
			jmp		do_sys_call
sleep:
			movl	$SYS_SLEEP,%eax
			jmp		do_sys_call
get_thrd_id:
			movl	$SYS_GET_THRD_ID,%eax
			jmp		do_sys_call
get_errno:
			movl	$SYS_GET_ERRNO,%eax
			jmp		do_sys_call
set_errno:
			movl	$SYS_SET_ERRNO,%eax
			jmp		do_sys_call

//Mutex
create_mutex:
			movl	$SYS_CREATE_MUTEX,%eax
			jmp		do_sys_call
acqr_mutex:
			movl	$SYS_ACQR_MUTEX,%eax
			jmp		do_sys_call
try_mutex:
			movl	$SYS_TRY_MUTEX,%eax
			jmp		do_sys_call
rls_mutex:
			movl	$SYS_RLS_MUTEX,%eax
			jmp		do_sys_call

//Semaphore
create_semaphore:
			movl	$SYS_CREATE_SEMAPHORE,%eax
			jmp		do_sys_call
acqr_semaphore:
			movl	$SYS_ACQR_SEMAPHORE,%eax
			jmp		do_sys_call
try_semaphore:
			movl	$SYS_TRY_SEMAPHORE,%eax
			jmp		do_sys_call
rls_semaphore:
			movl	$SYS_RLS_SEMAPHORE,%eax
			jmp		do_sys_call

//Message
recv_msg:
			movl	$SYS_RECV_MSG,%eax
			jmp		do_sys_call
complete_msg:
			movl	$SYS_COMPLETE_MSG,%eax
			jmp		do_sys_call
forward_msg:
			movl	$SYS_FORWARD_MSG,%eax
			jmp		do_sys_call
cancel_msg:
			movl	$SYS_CANCEL_MSG,%eax
			jmp		do_sys_call

//IO
read_port:
			movl	$SYS_READ_PORT,%eax
			jmp		do_sys_call
write_port:
			movl	$SYS_WRITE_PORT,%eax
			jmp		do_sys_call
get_tickcount:
			movl	$SYS_GET_TICKCOUNT,%eax
			jmp		do_sys_call
get_tick:
			movl	$SYS_GET_TICK,%eax
			jmp		do_sys_call
set_int_msg:
			movl	$SYS_SET_INT_MSG,%eax
			jmp		do_sys_call
clean_int_msg:
			movl	$SYS_CLEAN_INT_MSG,%eax
			jmp		do_sys_call
kprint:
			movl	$SYS_KPRINT,%eax
			jmp		do_sys_call
