/*
	  Copyright 2016,暗夜幽灵 <darknightghost.cn@gmail.com>

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

/*
      Header of kernel image,It will be called by lilo and initialize the
	running environment of kernel.
*/

#include "../../../../common/version.h"

.section	.text 
.global		_start

.equ		BASE_ADDR,		0x7C00
.equ		SECTOR_SIZE,	512

.code16
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
	
//Variables
error_message:
    .asciz  "You should use a boot loader program to load this kernel!"

_code_start:
	movw	%cs,%ax
	movw	%ax,%ds
	movw	%ax,%es
	call	cls
	pushw	$0x0F
	pushw	$0
	pushw	$0
	pushw	$error_message
	call	print_at_pos
loop:
	jmp		loop

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


//void		print_at_pos(
//				char* str,
//				unsigned short line,
//				unsigned short row,
//				unsigned char color);
print_at_pos:
	pushw	%bp
	movw	%sp,%bp
	//0x04(%bp):str
	//0x06(%bp):line
	//0x08(%bp):row
	//0x0A(%bp):color
	pushw	%bx
	pushw	%cx
	pushw	%dx
	movw	0x04(%bp),%ax
	pushw	%ax
	call	strlen
	add		$2,%sp
	movw	%ax,%cx
	movw	$0x1301,%ax
	xorw	%bx,%bx
	movb	0x0A(%bp),%bl
	movb	0x06(%bp),%dh
	movb	0x08(%bp),%dl
	pushw	%bp
	movw	0x04(%bp),%bp
	int		$0x10
	popw	%bp
	popw	%dx
	popw	%cx
	popw	%bx
	movw	%bp,%sp
	popw	%bp
	ret

//unsigned short strlen(char* str);
strlen:
	pushw	%bp
	movw	%sp,%bp
	//0x04(%bp):str
	pushw	%cx
	pushw	%di
	xorw	%ax,%ax
	movw	0x04(%bp),%di
	movw	$0xFFFF,%cx
	repnz	scasb
	movw	$0xFFFF,%ax
	subw	%cx,%ax
	decw	%ax
	popw	%di
	popw	%cx
	movw	%bp,%sp
	popw	%bp
	ret

.org	0x1f0,0xCC

//The real-mode kernel header of linux
setup_sects:	.byte	0			/* Filled in by build.c */
root_flags:		.word	1
syssize:		.long	0			/* Filled in by build.c */
ram_size:		.word	0			/* Obsolete */
vid_mode:		.word	0
root_dev:		.word	0			/* Filled in by build.c */
boot_flag:		.word	0xAA55
//jmp
_start:
				.byte	0xeb		# short (2-byte) jump
				.byte	0			#start_of_setup - 0x01
//header
		.ascii	"HdrS"		# header signature
//Version
		.word	0x0202		# header version number (>= 0x0105)
//realmode_swtch
		.word	0,0
//start_sys_seg
		.word	0x1000
//kernel_version
		.word	0
//type_of_loader
		.byte	0
//loadflags
		.byte	0x0001
//setup_move_size
		.word	0x8000
//code32_start
		.long	0x100000
ramdisk_image:
		.long	0
ramdisk_size:
		.long	0
//bootsect_kludge
		.long	0
heap_end_ptr:
		.word	_end + 1024 - 0x0200
//ext_loader_ver
		.byte	0
//ext_loader_type
		.byte	0
cmd_line_ptr:
		.long	0

//Variables
ver_str:
		.asciz	VER_STR
//Setup
start_of_setup:
	jmp		start_of_setup

//Extract kernel
//Enter protect mode
//Jmp to kernel
_end:
