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

#ifndef	SYSCALLS_H_INCLUDE
#define	SYSCALLS_H_INCLUDE

//Filesystem
#define	SYS_OPEN					0x00000000
#define	SYS_FCHMOD					0x00000001
#define	SYS_ACCESS					0x00000002
#define	SYS_CLOSE					0x00000003
#define	SYS_READ					0x00000004
#define	SYS_WRITE					0x00000005
#define	SYS_SEEK					0x00000006
#define	SYS_STAT					0x00000007
#define	SYS_UNLINK					0x00000008
#define	SYS_MKDIR					0x00000009
#define	SYS_READDIR					0x0000000A

//Mount
#define	SYS_MOUNT					0x00000100
#define	SYS_UMOUNT					0x00000101

//Objects
#define	SYS_CREATE_FLIE_OBJ			0x00000200
#define	SYS_CREATE_DRV_OBJ			0x00000201
#define	SYS_CREATE_DEV				0x00000202
#define	SYS_ADD_DEV					0x00000203
#define	SYS_REMOVE_DEV				0x00000204
#define	SYS_SET_DEV_FILENAME		0x00000205
#define	SYS_GET_MAJOR				0x00000206
#define	SYS_SYNC					0x00000207

//Memory
#define	SYS_VIRT_ALLOC				0x00000300
#define	SYS_VIRT_FREE				0x00000301
#define	SYS_MAP_PMO					0x00000302
#define	SYS_UNMAP_PMO				0x00000303
#define	SYS_MAP_RESERV_MEM			0x00000304
#define	SYS_UNMAP_RESERV_MEM			0x00000305

//Process
#define	SYS_FORK					0x00000400
#define	SYS_EXECVE					0x00000401
#define	SYS_WAITPID					0x00000402
#define	SYS_GET_PROC_ID				0x00000403
#define	SYS_GET_UID					0x00000404
#define	SYS_SET_UID					0x00000405
#define	SYS_GET_GID					0x00000406
#define	SYS_SET_GID					0x00000407
#define	SYS_CHG_TO_USR				0x00000408

//Thread
#define	SYS_SCHEDULE				0x00000500
#define	SYS_CREATE_THRD				0x00000501
#define	SYS_EXIT_THRD				0x00000502
#define	SYS_SUSPEND					0x00000503
#define	SYS_JOIN					0x00000504
#define	SYS_RESUME					0x00000505
#define	SYS_SLEEP					0x00000506
#define	SYS_GET_THRD_ID				0x00000507
#define	SYS_GET_ERRNO				0x00000508
#define	SYS_SET_ERRNO				0x00000509

//Mutex
#define	SYS_CREATE_MUTEX			0x00000600
#define	SYS_ACQR_MUTEX				0x00000601
#define	SYS_TRY_MUTEX				0x00000602
#define	SYS_RLS_MUTEX				0x00000603

//Semaphore
#define	SYS_CREATE_SEMAPHORE		0x00000610
#define	SYS_ACQR_SEMAPHORE			0x00000611
#define	SYS_TRY_SEMAPHORE			0x00000612
#define	SYS_RLS_SEMAPHORE			0x00000613

//Message
#define	SYS_RECV_MSG				0x00000700
#define	SYS_COMPLETE_MSG			0x00000701
#define	SYS_FORWARD_MSG				0x00000702
#define	SYS_CANCEL_MSG				0x00000703

//IO
#define	SYS_READ_PORT				0x00000800
#define	SYS_WRITE_PORT				0x00000801
#define	SYS_READ_PORT_DATAS			0x00000802
#define	SYS_WRITE_PORT_DATAS		0x00000803
#define	SYS_GET_TICKCOUNT			0x00000804
#define	SYS_GET_TICK				0x00000805
#define	SYS_SET_INT_MSG				0x00000806
#define	SYS_CLEAN_INT_MSG			0x00000807
#define	SYS_KPRINT					0x00000808

#endif	//!	SYSCALLS_H_INCLUDE
