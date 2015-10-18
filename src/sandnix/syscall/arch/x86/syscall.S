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

/*
	When a system call happens.
	EAX	:	System call number
	EBX	:	Start address of arguments
	ECX	:	User ESP
	EDX	:	User EIP
*/

#include "../../../setup/setup.h"
#include "../../syscall.h"

.section	.text
.global		syscall_init
.global		sys_call

#define	IA32_SYSENTER_CS	0x174
#define	IA32_SYSENTER_ESP	0x175
#define	IA32_SYSENTER_EIP	0x176

.code32
ssddt_addr:		.long		ssddt
ssudt_addr:		.long		ssudt
init_str:		.asciz		"\nInitializing System Calls...\n"

syscall_init:
		pushl	$init_str
		call	dbg_print
		addl	$4,%esp

		xorl	%edx,%edx
		movl	$SELECTOR_K_CODE,%eax
		movl	$IA32_SYSENTER_CS,%ecx
		wrmsr
		
		movl	$sys_call,%eax
		movl	$IA32_SYSENTER_EIP,%ecx
		wrmsr

		call	ssddt_init
		call	ssudt_init

		ret

sys_call:
		//EAX	:	System call number
		//EBX	:	Start address of arguments
		//ECX	:	User ESP
		//EDX	:	User EIP

		//Check system call number
		cmpl	$SYSCALL_MAX,%eax
		jb		_DO_SYS_CALL

			//Return 0
			xorl	%eax,%eax
			sysexit
		_DO_SYS_CALL:

		//Check if the process is a driver or a user process
		pushl	%ebx
		pushl	%eax
		pushl	%ecx
		pushl	%edx
		call	pm_is_driver
		cmp		0,%eax
		popl	%edx
		popl	%ecx
		popl	%eax

		//Get base address of system service table
		//Driver
		cmovnzl	ssddt_addr,%ebx
		//User process
		cmovzl	ssudt_addr,%ebx

		//Get start address of system service
		imul	$4,%eax
		addl	%ebx,%eax
		movl	(%eax),%eax

		//Check start address
		cmp		$0,%eax
		jne		_NOT_NULL
			xorl	%eax,%eax
			sysexit
		_NOT_NULL:

		//Do system call
		popl	%ebx
		pushl	%ecx
		pushl	%edx
		pushl	%ebx
		call	*%eax

		//Return
		addl	$4,%esp
		popl	%edx
		popl	%ecx

		sysexit
