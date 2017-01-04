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
		b		reset_hndlr

_undefined_entry:
		//Switch to SVC mode
		msr		cpsr_c, #0xD3

		//Space for PC
		//Space for cpsr
		sub		sp, sp, #8

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

		//Switch to undefined mode
		mov		r0, sp
		msr		cpsr_c,  #0xDB
		mov		sp, r0

		//Fill cpsr
		add		r0, sp, #(17 * 4)
		mrs		r1, spsr
		str		r1, [r0]

		//Fill pc
		add		r0, sp, #(18 * 4)
		str		lr, [r0]

		//Switch to system mode
		add		r0, sp, #(15 * 4)
		msr		cpsr_c, #0xDF
		stmfd	r0!, {sp}
		stmfd	r0!, {lr}

		//Switch to SVC mode
		msr		cpsr_c, #0xD3

		//Call handler
		mov		r0, sp
		b		undef_hndlr

_swi_entry:
		//PC
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

		//Switch to system mode
		add		r0, sp, #(15 * 4)
		msr		cpsr_c, #0xDF
		stmfd	r0!, {sp}
		stmfd	r0!, {lr}

		//Switch to SVC mode
		msr		cpsr_c, #0xD3

		//Call handler
		mov		r0, sp
		b		swi_hndlr

_prefetch_abort_entry:
		//Switch to SVC mode
		msr		cpsr_c, #0xD3

		//Space for PC
		//Space for cpsr
		sub		sp, sp, #8

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

		//Switch to abort mode
		mov		r0, sp
		msr		cpsr_c,  #0xD7
		mov		sp, r0

		//Fill cpsr
		add		r0, sp, #(17 * 4)
		mrs		r1, spsr
		str		r1, [r0]

		//Fill pc
		add		r0, sp, #(18 * 4)

		//if (spsr.T == 0) {
		mrs		r2, spsr
		ldr		r3, =0x20
		tst		r2, r3
			subeq		r1, lr, #4
		//} else {
			subne		r1, lr, #2
		//}

		str		r1, [r0]

		//Switch to system mode
		add		r0, sp, #(15 * 4)
		msr		cpsr_c, #0xDF
		stmfd	r0!, {sp}
		stmfd	r0!, {lr}

		//Switch to SVC mode
		msr		cpsr_c, #0xD3

		//Call handler
		mov		r0, sp
		b		prefetch_abort_hndlr

_data_abort_entry:
		//Switch to SVC mode
		msr		cpsr_c, #0xD3

		//Space for PC
		//Space for cpsr
		sub		sp, sp, #8

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

		//Switch to abort mode
		mov		r0, sp
		msr		cpsr_c,  #0xD7
		mov		sp, r0

		//Fill cpsr
		add		r0, sp, #(17 * 4)
		mrs		r1, spsr
		str		r1, [r0]

		//Fill pc
		add		r0, sp, #(18 * 4)

		//if (spsr.T == 0) {
		mrs		r2, spsr
		ldr		r3, =0x20
		tst		r2, r3
			subeq		r1, lr, #8
		//} else {
			subne		r1, lr, #4
		//}

		str		r1, [r0]

		//Switch to system mode
		add		r0, sp, #(15 * 4)
		msr		cpsr_c, #0xDF
		stmfd	r0!, {sp}
		stmfd	r0!, {lr}

		//Switch to SVC mode
		msr		cpsr_c, #0xD3

		//Call handler
		mov		r0, sp
		b		data_abort_hndlr

_reserved_entry:
		b		reserved_hndlr

_irq_entry:
		//Switch to SVC mode
		msr		cpsr_c, #0xD3

		//Space for PC
		//Space for cpsr
		sub		sp, sp, #8

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

		//Switch to irq mode
		mov		r0, sp
		msr		cpsr_c,  #0xD2
		mov		sp, r0

		//Fill cpsr
		add		r0, sp, #(17 * 4)
		mrs		r1, spsr
		str		r1, [r0]

		//Fill pc
		add		r0, sp, #(18 * 4)

		//if (spsr.T == 0) {
		mrs		r2, spsr
		ldr		r3, =0x20
		tst		r2, r3
			subeq		r1, lr, #4
		//} else {
			subne		r1, lr, #2
		//}

		str		r1, [r0]

		//Switch to system mode
		add		r0, sp, #(15 * 4)
		msr		cpsr_c, #0xDF
		stmfd	r0!, {sp}
		stmfd	r0!, {lr}

		//Switch to SVC mode
		msr		cpsr_c, #0xD3

		//Call handler
		mov		r0, sp
		b		irq_hndlr

_fiq_entry:
		//Switch to SVC mode
		msr		cpsr_c, #0xD3

		//Space for PC
		//Space for cpsr
		sub		sp, sp, #8

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

		//Switch to fiq mode
		mov		r0, sp
		msr		cpsr_c,  #0xD1
		mov		sp, r0

		//Fill cpsr
		add		r0, sp, #(17 * 4)
		mrs		r1, spsr
		str		r1, [r0]

		//Fill pc
		add		r0, sp, #(18 * 4)

		//if (spsr.T == 0) {
		mrs		r2, spsr
		ldr		r3, =0x20
		tst		r2, r3
			subeq		r1, lr, #4
		//} else {
			subne		r1, lr, #2
		//}

		str		r1, [r0]

		//Switch to system mode
		add		r0, sp, #(15 * 4)
		msr		cpsr_c, #0xDF
		stmfd	r0!, {sp}
		stmfd	r0!, {lr}

		//Switch to SVC mode
		msr		cpsr_c, #0xD3

		//Call handler
		mov		r0, sp
		b		irq_hndlr
