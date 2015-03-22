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

.equ		EXCEPTION_UNKNOW_EXCEPTION,0x00000000
.equ		EXCEPTION_HEAP_CORRUPTION,0x00000001
.equ		EXCEPTION_DE,0x00000002
.equ		EXCEPTION_DB,0x00000003
.equ		EXCEPTION_NMI,0x00000004
.equ		EXCEPTION_BR,0x00000005
.equ		EXCEPTION_UD,0x00000006
.equ		EXCEPTION_NM,0x00000007
.equ		EXCEPTION_DF,0x00000008
.equ		EXCEPTION_FPU,0x00000009
.equ		EXCEPTION_TS,0x0000000A
.equ		EXCEPTION_NP,0x0000000B
.equ		EXCEPTION_SS,0x0000000C
.equ		EXCEPTION_GP,0x0000000D
.equ		EXCEPTION_PF,0x0000000E
.equ		EXCEPTION_RESERVED,0x0000000F
.equ		EXCEPTION_MF,0x00000010
.equ		EXCEPTION_AC,0x00000011
.equ		EXCEPTION_MC,0x00000012
.equ		EXCEPTION_XF,0x00000013
