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

#include "../../../exceptions/err.h"

.global	de_int_handler
.global	db_int_handler
.global	nmi_int_handler
.global	bp_int_handler
.global	of_int_handler
.global	br_int_handler
.global	ud_int_handler
.global	nm_int_handler
.global	df_int_handler
.global	fpu_int_handler
.global	ts_int_handler
.global	np_int_handler
.global	ss_int_handler
.global	gp_int_handler
.global	pf_int_handler
.global	reserved_int_handler
.global	mf_int_handler
.global	ac_int_handler
.global	mc_int_handler
.global	xf_int_handler
.global	default_int_handler

fmt_df:
    .asciz  "Exception eip = %P"

.macro		NORMAL_INT_HNDLR num
	.global		int_\num
	int_\num :
		pushal
		pushl	%esp
		pushl	$\num
		call	int_normal_dispatcher
.endm

.macro		CLOCK_INT_HNDLR num
	.global		int_\num
	int_\num :
		pushal
		pushl	%esp
		pushl	$\num
		movl	tick_count,%eax
		incl	%eax
		movl	%eax,tick_count
		call	int_normal_dispatcher
.endm

.macro		BP_INT_HNDLR
		pushal
		pushl	%esp
		call	int_bp_dispatcher
.endm

.macro		EXCEPTION_INT_HNDLR num has_err_code
		pushal
		movb	exception_handling_flag,%al
		#if(exception_handling_flag)
		cmpb	$0,%al
		je		checked_\num
			addl	$(4+4*8),%esp
			movl	\has_err_code,%eax
			imull	$4,%eax
			addl	%eax,%esp
			#excpt_panic(EXCEPTION_DOUBLE_FAULT,fmt_df,%esp)
			pushl	%esp
			pushl	$fmt_df
			pushl	$0x00000019
			call	excpt_panic
	checked_\num:
		movb	$1,%al
		movb	%al,exception_handling_flag
		pushl	%esp
		pushl	$\num
		call	int_excpt_dispatcher
.endm

.section	.text
.code32

de_int_handler:
	EXCEPTION_INT_HNDLR	0x00,0
db_int_handler:
	EXCEPTION_INT_HNDLR	0x01,0
nmi_int_handler:
	EXCEPTION_INT_HNDLR	0x02,0
bp_int_handler:
	BP_INT_HNDLR
of_int_handler:
	EXCEPTION_INT_HNDLR	0x04,0
br_int_handler:
	EXCEPTION_INT_HNDLR	0x05,0
ud_int_handler:
	EXCEPTION_INT_HNDLR	0x06,0
nm_int_handler:
	EXCEPTION_INT_HNDLR	0x07,0
df_int_handler:
	EXCEPTION_INT_HNDLR	0x08,1
fpu_int_handler:
	EXCEPTION_INT_HNDLR	0x09,0
ts_int_handler:
	EXCEPTION_INT_HNDLR	0x0A,1
np_int_handler:
	EXCEPTION_INT_HNDLR	0x0B,1
ss_int_handler:
	EXCEPTION_INT_HNDLR	0x0C,1
gp_int_handler:
	EXCEPTION_INT_HNDLR	0x0D,1
pf_int_handler:
	EXCEPTION_INT_HNDLR	0x0E,1
reserved_int_handler:
	EXCEPTION_INT_HNDLR	0x0F,0
mf_int_handler:
	EXCEPTION_INT_HNDLR	0x10,0
ac_int_handler:
	EXCEPTION_INT_HNDLR	0x11,1
mc_int_handler:
	EXCEPTION_INT_HNDLR	0x12,0
xf_int_handler:
	EXCEPTION_INT_HNDLR	0x13,0
NORMAL_INT_HNDLR	0x14
NORMAL_INT_HNDLR	0x15
NORMAL_INT_HNDLR	0x16
NORMAL_INT_HNDLR	0x17
NORMAL_INT_HNDLR	0x18
NORMAL_INT_HNDLR	0x19
NORMAL_INT_HNDLR	0x1A
NORMAL_INT_HNDLR	0x1B
NORMAL_INT_HNDLR	0x1C
NORMAL_INT_HNDLR	0x1D
NORMAL_INT_HNDLR	0x1E
NORMAL_INT_HNDLR	0x1F

CLOCK_INT_HNDLR		0x20
NORMAL_INT_HNDLR	0x21
NORMAL_INT_HNDLR	0x22
NORMAL_INT_HNDLR	0x23
NORMAL_INT_HNDLR	0x24
NORMAL_INT_HNDLR	0x25
NORMAL_INT_HNDLR	0x26
NORMAL_INT_HNDLR	0x27
NORMAL_INT_HNDLR	0x28
NORMAL_INT_HNDLR	0x29
NORMAL_INT_HNDLR	0x2A
NORMAL_INT_HNDLR	0x2B
NORMAL_INT_HNDLR	0x2C
NORMAL_INT_HNDLR	0x2D
NORMAL_INT_HNDLR	0x2E
NORMAL_INT_HNDLR	0x2F

NORMAL_INT_HNDLR	0x30
NORMAL_INT_HNDLR	0x31
NORMAL_INT_HNDLR	0x32
NORMAL_INT_HNDLR	0x33
NORMAL_INT_HNDLR	0x34
NORMAL_INT_HNDLR	0x35
NORMAL_INT_HNDLR	0x36
NORMAL_INT_HNDLR	0x37
NORMAL_INT_HNDLR	0x38
NORMAL_INT_HNDLR	0x39
NORMAL_INT_HNDLR	0x3A
NORMAL_INT_HNDLR	0x3B
NORMAL_INT_HNDLR	0x3C
NORMAL_INT_HNDLR	0x3D
NORMAL_INT_HNDLR	0x3E
NORMAL_INT_HNDLR	0x3F

NORMAL_INT_HNDLR	0x40
NORMAL_INT_HNDLR	0x41
NORMAL_INT_HNDLR	0x42
NORMAL_INT_HNDLR	0x43
NORMAL_INT_HNDLR	0x44
NORMAL_INT_HNDLR	0x45
NORMAL_INT_HNDLR	0x46
NORMAL_INT_HNDLR	0x47
NORMAL_INT_HNDLR	0x48
NORMAL_INT_HNDLR	0x49
NORMAL_INT_HNDLR	0x4A
NORMAL_INT_HNDLR	0x4B
NORMAL_INT_HNDLR	0x4C
NORMAL_INT_HNDLR	0x4D
NORMAL_INT_HNDLR	0x4E
NORMAL_INT_HNDLR	0x4F

NORMAL_INT_HNDLR	0x50
NORMAL_INT_HNDLR	0x51
NORMAL_INT_HNDLR	0x52
NORMAL_INT_HNDLR	0x53
NORMAL_INT_HNDLR	0x54
NORMAL_INT_HNDLR	0x55
NORMAL_INT_HNDLR	0x56
NORMAL_INT_HNDLR	0x57
NORMAL_INT_HNDLR	0x58
NORMAL_INT_HNDLR	0x59
NORMAL_INT_HNDLR	0x5A
NORMAL_INT_HNDLR	0x5B
NORMAL_INT_HNDLR	0x5C
NORMAL_INT_HNDLR	0x5D
NORMAL_INT_HNDLR	0x5E
NORMAL_INT_HNDLR	0x5F

NORMAL_INT_HNDLR	0x60
NORMAL_INT_HNDLR	0x61
NORMAL_INT_HNDLR	0x62
NORMAL_INT_HNDLR	0x63
NORMAL_INT_HNDLR	0x64
NORMAL_INT_HNDLR	0x65
NORMAL_INT_HNDLR	0x66
NORMAL_INT_HNDLR	0x67
NORMAL_INT_HNDLR	0x68
NORMAL_INT_HNDLR	0x69
NORMAL_INT_HNDLR	0x6A
NORMAL_INT_HNDLR	0x6B
NORMAL_INT_HNDLR	0x6C
NORMAL_INT_HNDLR	0x6D
NORMAL_INT_HNDLR	0x6E
NORMAL_INT_HNDLR	0x6F

NORMAL_INT_HNDLR	0x70
NORMAL_INT_HNDLR	0x71
NORMAL_INT_HNDLR	0x72
NORMAL_INT_HNDLR	0x73
NORMAL_INT_HNDLR	0x74
NORMAL_INT_HNDLR	0x75
NORMAL_INT_HNDLR	0x76
NORMAL_INT_HNDLR	0x77
NORMAL_INT_HNDLR	0x78
NORMAL_INT_HNDLR	0x79
NORMAL_INT_HNDLR	0x7A
NORMAL_INT_HNDLR	0x7B
NORMAL_INT_HNDLR	0x7C
NORMAL_INT_HNDLR	0x7D
NORMAL_INT_HNDLR	0x7E
NORMAL_INT_HNDLR	0x7F

NORMAL_INT_HNDLR	0x80
NORMAL_INT_HNDLR	0x81
NORMAL_INT_HNDLR	0x82
NORMAL_INT_HNDLR	0x83
NORMAL_INT_HNDLR	0x84
NORMAL_INT_HNDLR	0x85
NORMAL_INT_HNDLR	0x86
NORMAL_INT_HNDLR	0x87
NORMAL_INT_HNDLR	0x88
NORMAL_INT_HNDLR	0x89
NORMAL_INT_HNDLR	0x8A
NORMAL_INT_HNDLR	0x8B
NORMAL_INT_HNDLR	0x8C
NORMAL_INT_HNDLR	0x8D
NORMAL_INT_HNDLR	0x8E
NORMAL_INT_HNDLR	0x8F

NORMAL_INT_HNDLR	0x90
NORMAL_INT_HNDLR	0x91
NORMAL_INT_HNDLR	0x92
NORMAL_INT_HNDLR	0x93
NORMAL_INT_HNDLR	0x94
NORMAL_INT_HNDLR	0x95
NORMAL_INT_HNDLR	0x96
NORMAL_INT_HNDLR	0x97
NORMAL_INT_HNDLR	0x98
NORMAL_INT_HNDLR	0x99
NORMAL_INT_HNDLR	0x9A
NORMAL_INT_HNDLR	0x9B
NORMAL_INT_HNDLR	0x9C
NORMAL_INT_HNDLR	0x9D
NORMAL_INT_HNDLR	0x9E
NORMAL_INT_HNDLR	0x9F

NORMAL_INT_HNDLR	0xA0
NORMAL_INT_HNDLR	0xA1
NORMAL_INT_HNDLR	0xA2
NORMAL_INT_HNDLR	0xA3
NORMAL_INT_HNDLR	0xA4
NORMAL_INT_HNDLR	0xA5
NORMAL_INT_HNDLR	0xA6
NORMAL_INT_HNDLR	0xA7
NORMAL_INT_HNDLR	0xA8
NORMAL_INT_HNDLR	0xA9
NORMAL_INT_HNDLR	0xAA
NORMAL_INT_HNDLR	0xAB
NORMAL_INT_HNDLR	0xAC
NORMAL_INT_HNDLR	0xAD
NORMAL_INT_HNDLR	0xAE
NORMAL_INT_HNDLR	0xAF

NORMAL_INT_HNDLR	0xB0
NORMAL_INT_HNDLR	0xB1
NORMAL_INT_HNDLR	0xB2
NORMAL_INT_HNDLR	0xB3
NORMAL_INT_HNDLR	0xB4
NORMAL_INT_HNDLR	0xB5
NORMAL_INT_HNDLR	0xB6
NORMAL_INT_HNDLR	0xB7
NORMAL_INT_HNDLR	0xB8
NORMAL_INT_HNDLR	0xB9
NORMAL_INT_HNDLR	0xBA
NORMAL_INT_HNDLR	0xBB
NORMAL_INT_HNDLR	0xBC
NORMAL_INT_HNDLR	0xBD
NORMAL_INT_HNDLR	0xBE
NORMAL_INT_HNDLR	0xBF

NORMAL_INT_HNDLR	0xC0
NORMAL_INT_HNDLR	0xC1
NORMAL_INT_HNDLR	0xC2
NORMAL_INT_HNDLR	0xC3
NORMAL_INT_HNDLR	0xC4
NORMAL_INT_HNDLR	0xC5
NORMAL_INT_HNDLR	0xC6
NORMAL_INT_HNDLR	0xC7
NORMAL_INT_HNDLR	0xC8
NORMAL_INT_HNDLR	0xC9
NORMAL_INT_HNDLR	0xCA
NORMAL_INT_HNDLR	0xCB
NORMAL_INT_HNDLR	0xCC
NORMAL_INT_HNDLR	0xCD
NORMAL_INT_HNDLR	0xCE
NORMAL_INT_HNDLR	0xCF

NORMAL_INT_HNDLR	0xD0
NORMAL_INT_HNDLR	0xD1
NORMAL_INT_HNDLR	0xD2
NORMAL_INT_HNDLR	0xD3
NORMAL_INT_HNDLR	0xD4
NORMAL_INT_HNDLR	0xD5
NORMAL_INT_HNDLR	0xD6
NORMAL_INT_HNDLR	0xD7
NORMAL_INT_HNDLR	0xD8
NORMAL_INT_HNDLR	0xD9
NORMAL_INT_HNDLR	0xDA
NORMAL_INT_HNDLR	0xDB
NORMAL_INT_HNDLR	0xDC
NORMAL_INT_HNDLR	0xDD
NORMAL_INT_HNDLR	0xDE
NORMAL_INT_HNDLR	0xDF

NORMAL_INT_HNDLR	0xE0
NORMAL_INT_HNDLR	0xE1
NORMAL_INT_HNDLR	0xE2
NORMAL_INT_HNDLR	0xE3
NORMAL_INT_HNDLR	0xE4
NORMAL_INT_HNDLR	0xE5
NORMAL_INT_HNDLR	0xE6
NORMAL_INT_HNDLR	0xE7
NORMAL_INT_HNDLR	0xE8
NORMAL_INT_HNDLR	0xE9
NORMAL_INT_HNDLR	0xEA
NORMAL_INT_HNDLR	0xEB
NORMAL_INT_HNDLR	0xEC
NORMAL_INT_HNDLR	0xED
NORMAL_INT_HNDLR	0xEE
NORMAL_INT_HNDLR	0xEF

NORMAL_INT_HNDLR	0xF0
NORMAL_INT_HNDLR	0xF1
NORMAL_INT_HNDLR	0xF2
NORMAL_INT_HNDLR	0xF3
NORMAL_INT_HNDLR	0xF4
NORMAL_INT_HNDLR	0xF5
NORMAL_INT_HNDLR	0xF6
NORMAL_INT_HNDLR	0xF7
NORMAL_INT_HNDLR	0xF8
NORMAL_INT_HNDLR	0xF9
NORMAL_INT_HNDLR	0xFA
NORMAL_INT_HNDLR	0xFB
NORMAL_INT_HNDLR	0xFC
NORMAL_INT_HNDLR	0xFD
NORMAL_INT_HNDLR	0xFE
NORMAL_INT_HNDLR	0xFF

