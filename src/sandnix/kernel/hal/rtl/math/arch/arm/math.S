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

.global	__aeabi_uidiv
.global __aeabi_uidivmod

//r0 : divident
//r1 : divisor
//Return value is in r0
__aeabi_uidiv:
	ldr		r2, =hal_rtl_math_div32
	stmfd	sp!, {lr}
	blx		r2
	ldmfd	sp!, {lr}
	mov		pc, lr

//r0 : divident
//r1 : divisor
//Return value is in r1
__aeabi_uidivmod:
	ldr		r2, =hal_rtl_math_mod32
	stmfd	sp!, {lr}
	blx		r2
	ldmfd	sp!, {lr}
	mov		r1, r0
	mov		pc, lr
