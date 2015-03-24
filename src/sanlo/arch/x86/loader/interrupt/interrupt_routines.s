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
.include	"exception/exception.inc"

.equ		LOADER_OFFSET,0x00030000

.section	.text
.code32

//-----------------------------Global functions---------------------------------
.global		setup_idt
.global		int_00
.global		int_01
.global		int_02
.global		int_05
.global		int_06
.global		int_07
.global		int_08
.global		int_09
.global		int_0A
.global		int_0B
.global		int_0C
.global		int_0D
.global		int_0E
.global		int_0F
.global		int_10
.global		int_11
.global		int_12
.global		int_13

.global		int_21

//--------------------------------Macros----------------------------------------

//---------------------------------Codes----------------------------------------
		
int_00:
		pushl	$EXCEPTION_DE
		call	panic

int_01:
		pushl	$EXCEPTION_DB
		call	panic

int_02:
		pushl	$EXCEPTION_NMI
		call	panic

int_05:
		pushl	$EXCEPTION_BR
		call	panic

int_06:
		pushl	$EXCEPTION_UD
		call	panic

int_07:
		pushl	$EXCEPTION_NM
		call	panic

int_08:
		pushl	$EXCEPTION_DF
		call	panic

int_09:
		pushl	$EXCEPTION_FPU
		call	panic

int_0A:
		pushl	$EXCEPTION_TS
		call	panic

int_0B:
		pushl	$EXCEPTION_NP
		call	panic

int_0C:
		pushl	$EXCEPTION_SS
		call	panic

int_0D:
		pushl	$EXCEPTION_GP
		call	panic

int_0E:
		pushl	$EXCEPTION_PF
		call	panic

int_0F:
		pushl	$EXCEPTION_RESERVED
		call	panic

int_10:
		pushl	$EXCEPTION_MF
		call	panic

int_11:
		pushl	$EXCEPTION_AC
		call	panic

int_12:
		pushl	$EXCEPTION_MC
		call	panic

int_13:
		pushl	$EXCEPTION_XF
		call	panic

