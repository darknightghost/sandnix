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

.section	.text
.global		ivt_base

#include "../../../../../../mmu/mmu.h"

.align		16, 0
ivt_base:
	ldr		pc, _reset_entry
	b		_undefined_entry
	ldr		pc, _swi_entry
	ldr		pc, _prefetch_abort_entry
	ldr		pc, _data_abort_entry
	ldr		pc, _reserved_entry
	ldr		pc, _irq_entry
	ldr		pc, _fiq_entry


.align		4, 0
//Interrupt handler entry
_reset_entry:
		b		_reset_entry

_undefined_entry:
		//Switch to SVC mode
		msr		cpsr_c,	#0xD3

		//Push context
		//r0-r12
		stmfd	sp!, {r0-r12}

1:
b		1b

		//sp, lr, pc
		mov		r0, sp

		//Switch to undefined mode
		msr		cpsr_c,	#0xDB
		mov		sp, r0

		mov		r0, lr
		ldr		r1,	=KERNEL_MEM_BASE

		//if(lr >= KERNEL_MEM_BASE) 
		cmp		r0, r1
		1:
		blo		1f

		//{
			//The interrupt is called from kernel memory
			//sp
			mov		r0, sp
			add		r0, r0, #(13 * 4)
			stmfd	sp!, {r0}

			//lr pc
			mov		r0, sp
			mov		r1, lr

			//Switch to SVC mode
			msr		cpsr_c,	#0xD3
			mov		sp, r0
			stmfd	sp!, {lr}
 			stmfd	sp!, {r1}

		//} else {
			1:

			//The interrupt is called from user memory
			//Switch to system mode
			msr		cpsr_c,	#0xDF
			mov		r0, sp
			mov		r1, lr

			//Switch to SVC mode
			msr		cpsr_c,	#0xD3
			stmfd	sp!, {r0-r1}

			//Switch to undefined mode
			mov		r0, sp
			msr		cpsr_c,	#0xDB
			stmfd	r0!, {lr}

			//Switch to SVC mode
			msr		cpsr_c,	#0xD3
			mov		sp, r0

		//}

		mov		r0, #1

_swi_entry:
		b		_swi_entry

_prefetch_abort_entry:
		b		_prefetch_abort_entry

_data_abort_entry:
		b		_data_abort_entry

_reserved_entry:
		b		_reserved_entry

_irq_entry:
		b		_irq_entry

_fiq_entry:
		b		_fiq_entry
