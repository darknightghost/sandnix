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

//void hal_exception_raise(pexcept_obj_t p_exception);
hal_exception_raise:
		//(%esp) : eip
		//4(%esp) : p_exception

		//Push context
		pushal

		//%eax : old esp
		movl	16(%esp), %eax

		//%ebx : eip
		movl	(%eax), %ebx

		addl	$4, %eax
		movl	%eax, 16(%esp)	
		pushl	%ebx

		xorl	%eax, %eax
		//cs
		movl	$SELECTOR_K_CODE, %eax
		pushl	%eax

		//eflags
		pushfl

		//ss
		movw	%ss, %ax
		pushl	%eax

		//gs
		movw	%gs, %ax
		pushl	%eax

		//fs
		movw	%fs, %ax
		pushl	%eax

		//ds
		movw	%ds, %ax
		pushl	%eax

		//es
		movw	%es, %ax
		pushl	%eax

		subl	$108, %esp
		fnsave	(%esp)

		jmp		core_exception_raise
