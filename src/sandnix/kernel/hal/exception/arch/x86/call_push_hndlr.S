/*
   Copyright 2016,王思远 <darknightghost.cn@gmail.com>

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

#include "../../../init/init_defs.h"
#include "../../../../core/exception/exception_defs.h"

.section	.text
.global		hal_exception_raise

//bool hal_exception_hndlr_push_caller(pexcept_hndlr_info_t p_hndlr_info);
hal_exception_hndlr_push_caller:
	//Save returning address
	//eax = returning address
	movl	(%esp), %eax

	//ecx = esp
	movl	%esp, %ecx

	//Save context
	pushal

	//eip
	pushl	$_EXCEPT_RET

	//cs
	movl	%cs, %eax
	push	%eax

	//eflags
	pushfl

	//ss
	movl	%ss, %eax
	push	%eax

	//gs
	movl	%gs, %eax
	push	%eax

	//fs
	movl	%fs, %eax
	push	%eax

	//ds
	movl	%ds, %eax
	push	%eax

	//es
	movl	%es, %eax
	push	%eax

	//fpu environment
	subl	$108, %esp
	fnsave	(%esp)

	//Call core_exception_do_push_hndlr
	//p_context
	pushl	%esp

	//p_hndlr_info
	pushl	%ecx
	addl	$4, %ecx
	pushl	(%ecx)
	call	core_exception_do_push_hndlr
	addl	$8, %esp
	popl	%esp
	ret

	//Exception return
_EXCEPT_RET:
	movl	%eax, (%esp)
	movl	$EXCEPT_RET_UNWIND, %eax
	ret
