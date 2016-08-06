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

#include "../../init.h"

.global		_start
.global		kernel_header

.section	.text
.align		8
kernel_header:
	//magic
	.long	KERNEL_HEADER_MAGIC
	//code_start
	.long	0
	//code_size
	.long	0
	//data_start
	.long	0
	//data_size
	.long	0
	//header_size
	.long	_kernel_header_end - kernel_header
	//checksum
	.long	0x100000000 - ((KERNEL_HEADER_MAGIC + (_kernel_header_end - kernel_header)) & 0xFFFFFFFF)
_kernel_header_end:
	.long	0

_start:
	//Make sure we are in SVC mode
	ldr		r5,	=0x000000D3
	msr		spsr, r5
	msr		cpsr, r5

	//Compute offset
_offchk_addr:
	mov		r5, pc
	sub		r5, r5, #8
	ldr		r6, =_offchk_addr
	sub		r5, r5, r6		//r5 = offset

	//Prepare stack
	ldr		sp,	=init_stack
	add		sp, sp, r5
	ldr		r6,=DEFAULT_STACK_SIZE
	add		sp, sp, r6

#if BOOTLOADER == MULTIBOOT2

	//Push kernel args
	stmfd	sp!, {r2}
#endif

	//Start paging
	bl		start_paging

	//Relocate stack
	sub		sp, sp, r5

	//Call kinit
	ldmfd	sp!, {r0}
	ldr		r1,	=kinit
	mov		pc, r1
