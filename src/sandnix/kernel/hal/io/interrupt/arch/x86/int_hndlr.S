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

#include "../../../../init/init.h"

.section	.text
//esp = &context
.macro		PUSH_CONTEXT
		pushl	$0
		pushal

		mov		%esp, %ebx

		//Get eip
		addl	$(32 + 4), %ebx
		pushl	(%ebx)

		//Get cs
		addl	$4, %ebx
		//eax = cs
		movl	(%ebx), %eax
		pushl	%eax

		//Get eflags
		addl	$4, %ebx
		pushl	(%ebx)

		//if(%ax == SELECTOR_K_CODE)
		cmpw	$SELECTOR_K_CODE, %ax
		jne		1f
			//Kernel
			addl	$4,	%ebx
			movl	%ebx, 24(%esp)		//esp in context

			//ss
			pushl	%ss
		jmp		2f
		//else
		1:
			//User space
			addl	$4,	%ebx
			movl	(%ebx), %eax		
			movl	%eax, 24(%esp)		//esp in context

			//ss
			addl	$4,	%ebx
			pushl	(%ebx)
			
		2:
		pushl	%gs
		pushl	%fs
		pushl	%ds
		pushl	%es

		//Save fpu status
		subl	$108, %esp
		fnsave	(%esp)
.endm

//esp = &context
//eax = err_code
.macro		PUSH_CONTEXT_WITH_ERRCODE
		pushal
		mov		%esp, %ebx
		
		//Get error code
		addl	$32, %ebx
		movl	(%ebx), %edx

		//Get eip
		addl	$4, %ebx
		pushl	(%ebx)

		//Get cs
		addl	$4, %ebx
		//eax = cs
		movl	(%ebx), %eax
		pushl	%eax

		//Get eflags
		addl	$4, %ebx
		pushl	(%ebx)

		//if(%ax == SELECTOR_K_CODE)
		cmpw	$SELECTOR_K_CODE, %ax
		jne		1f
			//Kernel
			addl	$4,	%ebx
			movl	%ebx, 24(%esp)		//esp in context

			//ss
			pushl	%ss
		jmp		2f
		//else
		1:
			//User space
			addl	$4,	%ebx
			movl	(%ebx), %eax		
			movl	%eax, 24(%esp)		//esp in context

			//ss
			addl	$4,	%ebx
			pushl	(%ebx)
		2:
		pushl	%gs
		pushl	%fs
		pushl	%ds
		pushl	%es

		//Save fpu status
		subl	$108, %esp
		fnsave	(%esp)

		movl	%edx, %eax

.endm

.macro		NORMAL_IDT num
	.global	int_\num
	int_\num:
		//Save Registers
		PUSH_CONTEXT

		//Call dispatcher
		movl	%esp, %eax
		pushl	$0
		pushl	%eax
		pushl	$\num
		call	int_dispatcher
.endm

.macro		NORMAL_EXCEPT_IDT num
	.global	int_\num
	int_\num:
		//Save Registers
		PUSH_CONTEXT

		//Call dispatcher
		movl	%esp, %eax
		pushl	$0
		pushl	%eax
		pushl	$\num
		call	int_except_dispatcher
.endm

.macro		EXCEPT_IDT_WITH_ERRCODE num
	.global	int_\num
	int_\num:
		//Save Registers
		PUSH_CONTEXT_WITH_ERRCODE

		//Call dispatcher
		movl	%esp, %ebx
		pushl	%eax
		pushl	%ebx
		pushl	$\num
		call	int_except_dispatcher
.endm

.macro		RESERVED_IDT num
	.global	int_\num
	int_\num:
		jmp		int_\num
		iret
.endm

//Exceptions
NORMAL_EXCEPT_IDT			0x00
NORMAL_EXCEPT_IDT			0x01
NORMAL_EXCEPT_IDT			0x02
NORMAL_EXCEPT_IDT			0x03
NORMAL_EXCEPT_IDT			0x04
NORMAL_EXCEPT_IDT			0x05
NORMAL_EXCEPT_IDT			0x06
NORMAL_EXCEPT_IDT			0x07
NORMAL_EXCEPT_IDT			0x08
RESERVED_IDT				0x09
EXCEPT_IDT_WITH_ERRCODE		0x0A
EXCEPT_IDT_WITH_ERRCODE		0x0B
EXCEPT_IDT_WITH_ERRCODE		0x0C
EXCEPT_IDT_WITH_ERRCODE		0x0D
EXCEPT_IDT_WITH_ERRCODE		0x0E
RESERVED_IDT				0x0F
NORMAL_EXCEPT_IDT			0x10
EXCEPT_IDT_WITH_ERRCODE		0x11
NORMAL_EXCEPT_IDT			0x12
NORMAL_EXCEPT_IDT			0x13

//Reserved
RESERVED_IDT    0x14
RESERVED_IDT    0x15
RESERVED_IDT    0x16
RESERVED_IDT    0x17
RESERVED_IDT    0x18
RESERVED_IDT    0x19
RESERVED_IDT    0x1A
RESERVED_IDT    0x1B
RESERVED_IDT    0x1C
RESERVED_IDT    0x1D
RESERVED_IDT    0x1E
RESERVED_IDT    0x1F

//Normal interrupt
NORMAL_IDT      0x20
NORMAL_IDT      0x21
NORMAL_IDT      0x22
NORMAL_IDT      0x23
NORMAL_IDT      0x24
NORMAL_IDT      0x25
NORMAL_IDT      0x26
NORMAL_IDT      0x27
NORMAL_IDT      0x28
NORMAL_IDT      0x29
NORMAL_IDT      0x2A
NORMAL_IDT      0x2B
NORMAL_IDT      0x2C
NORMAL_IDT      0x2D
NORMAL_IDT      0x2E
NORMAL_IDT      0x2F
NORMAL_IDT      0x30
NORMAL_IDT      0x31
NORMAL_IDT      0x32
NORMAL_IDT      0x33
NORMAL_IDT      0x34
NORMAL_IDT      0x35
NORMAL_IDT      0x36
NORMAL_IDT      0x37
NORMAL_IDT      0x38
NORMAL_IDT      0x39
NORMAL_IDT      0x3A
NORMAL_IDT      0x3B
NORMAL_IDT      0x3C
NORMAL_IDT      0x3D
NORMAL_IDT      0x3E
NORMAL_IDT      0x3F
NORMAL_IDT      0x40
NORMAL_IDT      0x41
NORMAL_IDT      0x42
NORMAL_IDT      0x43
NORMAL_IDT      0x44
NORMAL_IDT      0x45
NORMAL_IDT      0x46
NORMAL_IDT      0x47
NORMAL_IDT      0x48
NORMAL_IDT      0x49
NORMAL_IDT      0x4A
NORMAL_IDT      0x4B
NORMAL_IDT      0x4C
NORMAL_IDT      0x4D
NORMAL_IDT      0x4E
NORMAL_IDT      0x4F
NORMAL_IDT      0x50
NORMAL_IDT      0x51
NORMAL_IDT      0x52
NORMAL_IDT      0x53
NORMAL_IDT      0x54
NORMAL_IDT      0x55
NORMAL_IDT      0x56
NORMAL_IDT      0x57
NORMAL_IDT      0x58
NORMAL_IDT      0x59
NORMAL_IDT      0x5A
NORMAL_IDT      0x5B
NORMAL_IDT      0x5C
NORMAL_IDT      0x5D
NORMAL_IDT      0x5E
NORMAL_IDT      0x5F
NORMAL_IDT      0x60
NORMAL_IDT      0x61
NORMAL_IDT      0x62
NORMAL_IDT      0x63
NORMAL_IDT      0x64
NORMAL_IDT      0x65
NORMAL_IDT      0x66
NORMAL_IDT      0x67
NORMAL_IDT      0x68
NORMAL_IDT      0x69
NORMAL_IDT      0x6A
NORMAL_IDT      0x6B
NORMAL_IDT      0x6C
NORMAL_IDT      0x6D
NORMAL_IDT      0x6E
NORMAL_IDT      0x6F
NORMAL_IDT      0x70
NORMAL_IDT      0x71
NORMAL_IDT      0x72
NORMAL_IDT      0x73
NORMAL_IDT      0x74
NORMAL_IDT      0x75
NORMAL_IDT      0x76
NORMAL_IDT      0x77
NORMAL_IDT      0x78
NORMAL_IDT      0x79
NORMAL_IDT      0x7A
NORMAL_IDT      0x7B
NORMAL_IDT      0x7C
NORMAL_IDT      0x7D
NORMAL_IDT      0x7E
NORMAL_IDT      0x7F
NORMAL_IDT      0x80
NORMAL_IDT      0x81
NORMAL_IDT      0x82
NORMAL_IDT      0x83
NORMAL_IDT      0x84
NORMAL_IDT      0x85
NORMAL_IDT      0x86
NORMAL_IDT      0x87
NORMAL_IDT      0x88
NORMAL_IDT      0x89
NORMAL_IDT      0x8A
NORMAL_IDT      0x8B
NORMAL_IDT      0x8C
NORMAL_IDT      0x8D
NORMAL_IDT      0x8E
NORMAL_IDT      0x8F
NORMAL_IDT      0x90
NORMAL_IDT      0x91
NORMAL_IDT      0x92
NORMAL_IDT      0x93
NORMAL_IDT      0x94
NORMAL_IDT      0x95
NORMAL_IDT      0x96
NORMAL_IDT      0x97
NORMAL_IDT      0x98
NORMAL_IDT      0x99
NORMAL_IDT      0x9A
NORMAL_IDT      0x9B
NORMAL_IDT      0x9C
NORMAL_IDT      0x9D
NORMAL_IDT      0x9E
NORMAL_IDT      0x9F
NORMAL_IDT      0xA0
NORMAL_IDT      0xA1
NORMAL_IDT      0xA2
NORMAL_IDT      0xA3
NORMAL_IDT      0xA4
NORMAL_IDT      0xA5
NORMAL_IDT      0xA6
NORMAL_IDT      0xA7
NORMAL_IDT      0xA8
NORMAL_IDT      0xA9
NORMAL_IDT      0xAA
NORMAL_IDT      0xAB
NORMAL_IDT      0xAC
NORMAL_IDT      0xAD
NORMAL_IDT      0xAE
NORMAL_IDT      0xAF
NORMAL_IDT      0xB0
NORMAL_IDT      0xB1
NORMAL_IDT      0xB2
NORMAL_IDT      0xB3
NORMAL_IDT      0xB4
NORMAL_IDT      0xB5
NORMAL_IDT      0xB6
NORMAL_IDT      0xB7
NORMAL_IDT      0xB8
NORMAL_IDT      0xB9
NORMAL_IDT      0xBA
NORMAL_IDT      0xBB
NORMAL_IDT      0xBC
NORMAL_IDT      0xBD
NORMAL_IDT      0xBE
NORMAL_IDT      0xBF
NORMAL_IDT      0xC0
NORMAL_IDT      0xC1
NORMAL_IDT      0xC2
NORMAL_IDT      0xC3
NORMAL_IDT      0xC4
NORMAL_IDT      0xC5
NORMAL_IDT      0xC6
NORMAL_IDT      0xC7
NORMAL_IDT      0xC8
NORMAL_IDT      0xC9
NORMAL_IDT      0xCA
NORMAL_IDT      0xCB
NORMAL_IDT      0xCC
NORMAL_IDT      0xCD
NORMAL_IDT      0xCE
NORMAL_IDT      0xCF
NORMAL_IDT      0xD0
NORMAL_IDT      0xD1
NORMAL_IDT      0xD2
NORMAL_IDT      0xD3
NORMAL_IDT      0xD4
NORMAL_IDT      0xD5
NORMAL_IDT      0xD6
NORMAL_IDT      0xD7
NORMAL_IDT      0xD8
NORMAL_IDT      0xD9
NORMAL_IDT      0xDA
NORMAL_IDT      0xDB
NORMAL_IDT      0xDC
NORMAL_IDT      0xDD
NORMAL_IDT      0xDE
NORMAL_IDT      0xDF
NORMAL_IDT      0xE0
NORMAL_IDT      0xE1
NORMAL_IDT      0xE2
NORMAL_IDT      0xE3
NORMAL_IDT      0xE4
NORMAL_IDT      0xE5
NORMAL_IDT      0xE6
NORMAL_IDT      0xE7
NORMAL_IDT      0xE8
NORMAL_IDT      0xE9
NORMAL_IDT      0xEA
NORMAL_IDT      0xEB
NORMAL_IDT      0xEC
NORMAL_IDT      0xED
NORMAL_IDT      0xEE
NORMAL_IDT      0xEF
NORMAL_IDT      0xF0
NORMAL_IDT      0xF1
NORMAL_IDT      0xF2
NORMAL_IDT      0xF3
NORMAL_IDT      0xF4
NORMAL_IDT      0xF5
NORMAL_IDT      0xF6
NORMAL_IDT      0xF7
NORMAL_IDT      0xF8
NORMAL_IDT      0xF9
NORMAL_IDT      0xFA
NORMAL_IDT      0xFB
NORMAL_IDT      0xFC
NORMAL_IDT      0xFD
NORMAL_IDT      0xFE
NORMAL_IDT      0xFF
