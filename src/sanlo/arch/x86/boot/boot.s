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

.include	"../common_defs.inc"
.section	.text 
.global		_start
//------------------------------Definitions-------------------------------------
.equ		BASE_ADDR,0x7C00
.equ		SECTOR_SIZE,512
//--------------------------------BPB----------------------------------------
.code16
_start:
		jmp		_code_start
		nop

//BPB
BS_OEMNAME:					
		.ascii	"SANDNIX\0"				#Must be 8 bytes
BPB_BytePerSec:
		.word		SECTOR_SIZE
BPB_SecPerClus:
		.byte		1
BPB_RsvdSecCnt:
		.word		1
BPB_NumFATs:
		.byte		2
BPB_RootEntCnt:
		.word		224
BPB_TotSec16:
		.word		2880
BPB_Media:
		.byte		0xF0
BPB_FATSz16:
		.word		9
BPB_SecPerTrk:
		.word		18
BPB_NumHeads:
		.word		2
BPB_HiddSec:
		.word		0
		.word		0
BPB_TotSec32:
		.word		0
		.word		0
BS_DrvNum:
		.byte		0
BS_Reserved1:
		.byte		0
BS_BootSig:
		.byte		0x29
BS_VolID:
		.word		0
		.word		0
BS_VolLab:
		.ascii	"SANDNIX\0\0\0\0"		#Must be 11 bytes
BS_FileSysType:
		.ascii	"SANLOIMG"				#Must be 8 bytes
//------------------------------Variables---------------------------------------
g_msg_search_kernel_loader:
    .asciz  "Loading kernel loader..."
//--------------------------------Codes-----------------------------------------
_code_start:
		movw	%cs,%ax
		movw	%ax,%ds
		movw	%ax,%es
		call	cls
		call	reset_floppy
		//Get sectors of loader
		call	load_sectors
		movw	$LOADER_SECTION,%ax
		movw	%ax,%es
		movw	%es:0x0000,%ax
		ljmp	$LOADER_SECTION,$0x0000
//------------------------------Functions--------------------------------------

//String functions
//void		cls();
cls:
		pushw	%bx
		pushw	%cx
		pushw	%dx
		movb	$0x06,%ah  
		movb	$0,%al  
		movb	$0,%cl
		movb	$0,%ch    
		movb	$24,%dh  
		movb	$79,%dl
		movb	$0x07,%bh
		int		$0x10
		popw	%dx
		popw	%cx
		popw	%bx
		ret  

//Floppy functions
//void		reset_floppy();
reset_floppy:
		pushw	%ax
		pushw	%dx
		xorw	%ax,%ax
		xorw	%dx,%dx
		int		$0x13
		popw	%dx
		popw	%ax
		ret

//void		load_sectors()

load_sectors:
		pushw	%bp
		movw	%sp,%bp
		pushw	%bx
		pushw	%cx
		pushw	%dx
		pushw	%es
		movb	$0,%ch
		movb	$2,%cl
		xorw	%dx,%dx
		movw	$LOADER_SECTION,%ax
		movw	%ax,%es
		xorw	%bx,%bx
		movb	$0x02,%ah
		movb	$LOADER_SECTORS,%al
_read_floppy:
		int		$0x13
		jc		_read_floppy
		popw	%es
		popw	%dx
		popw	%cx
		popw	%bx
		movw	%bp,%sp
		popw	%bp
		ret
.org		0x1fe,0xCC
.word		0xaa55  
