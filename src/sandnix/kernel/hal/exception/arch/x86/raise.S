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

.section	.text
.global		hal_exception_raise

//void hal_exception_raise(pexcept_obj_t p_exception, char* file, u32 line,
//                         char* comment);
hal_exception_raise:
	//Save context
	pop		%eax

	//ecx = esp
	movl	%esp, %ecx
	pushal

	//eip
	push	%eax

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

	//Call do_raise
	//p_context
	pushl	%esp

	//ebx = old esp
	movl	%ecx, %ebx

	//ebx = comment
	addl	$12, %ebx
	pushl	(%ebx)

	//line
	subl	$4, %ebx
	pushl	(%ebx)

	//file
	subl	$4, %ebx
	pushl	(%ebx)

	//p_exception
	subl	$4, %ebx
	pushl	(%ebx)

	call	do_raise

