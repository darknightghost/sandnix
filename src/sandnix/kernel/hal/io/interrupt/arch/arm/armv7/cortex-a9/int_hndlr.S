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

.align		16
ivt_base:
_ivt_reset:
	b		_reset_entry

_ivt_undefined:
	b		_undefined_entry

_ivt_swi:
	b		_swi_entry

_ivt_prefetch_abort:
	b		_prefetch_abort_entry

_ivt_data_abort:
	b		_data_abort_entry

_ivt_reserved:
	b		_reserved_entry

_ivt_irq:
	b		_irq_entry

_ivt_fiq:
	b		_fiq_entry


.align		4, 0
//Interrupt handler entry
_reset_entry:
		b		_reset_entry

_undefined_entry:
		msr		cpsr_c, #0xD3
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		b	_undefined_entry


_swi_entry:
		//Space for PC
		//Space for cpsr
		stmfd	sp!, {lr}
		sub		sp, sp, #4

		//sp_svc
		//lr_svc
		stmfd	sp!, {sp}
		stmfd	sp!, {lr}

		//sp_usr
		//lr_usr
		sub		sp, sp, #8

		//Push r0 - r12
		stmfd	sp!, {r0 - r12}

		//Fix sp_svc value
		add		r0, sp, #(16 * 4)
		ldr		r1, [r0]
		add		r1, r1, #8
		str		r1, [r0]

		//Fill cpsr
		add		r0, sp, #(17 * 4)
		mrs		r1, spsr
		str		r1, [r0]

		add		r0, sp, #(15 * 4)

		//Switch to system mode
		msr		cpsr_c, #0xDF
		stmfd	r0!, {sp}
		stmfd	r0!, {lr}

		//Switch to SVC mode
		msr		cpsr_c, #0xD3

		//Call handler
		mov		r0, sp
		b		swi_hndlr

_prefetch_abort_entry:
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		b		_prefetch_abort_entry

_data_abort_entry:
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		b		_data_abort_entry

_reserved_entry:
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		b		_reserved_entry

_irq_entry:
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		b		_irq_entry

_fiq_entry:
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		b		_fiq_entry
