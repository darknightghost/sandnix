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

.include	"../common_defs.s"
.section	.text 

.global		_start
.global		print_at_pos
.comm		c_entry,0
//--------------------------------Macros----------------------------------------
.macro		SEGMENT_DESCRIPTOR base,limit,property
	.word		\limit & 0xFFFF									#Limit1 
	.word		\base & 0xFFFF									#Base1
	.byte		(\base >> 16) & 0xFF							#Base2
	.word		((\limit >> 8) & 0x0F00) | (\property & 0xF0FF)	#Property1+Limit2+property2
	.byte		(\base >> 24) & 0xFF							#Base3
.endm
//------------------------------Definitions-------------------------------------
.equ		LOADER_SIZE,LOADER_SECTORS*512
//---------------------------------Entry----------------------------------------
.code16
_start:
	jmp		_code_start
//------------------------------Variables---------------------------------------
msg_kernel_loader_loaded:
    .asciz  "Kernel loader loaded."
msg_entering_protect_mode:
    .asciz  "Entering protect mode..."

//---------------------------------GDT------------------------------------------
gdt:
gdt_normal_limit:
SEGMENT_DESCRIPTOR	0,				0,					0

//------------------------------Functions---------------------------------------

//String functions
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
//void		cls();
cls:
	pushw	%bx
	pushw	%cx
	pushw	%dx
	movb	$0x06,%ah  
    movb	$0,%al  
    movb	$0,%ch  
    movb	$0,%cl  
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
	
//--------------------------------Codes-----------------------------------------

_code_start:
    movw	%cs,%ax
    movw	%ax,%ds
    movw	%ax,%es
    pushw	$0x0c
    pushw	$0
    pushw	$1
    pushw	$msg_kernel_loader_loaded
    call	print_at_pos
    addw	$0x08,%sp
    pushw	$0x0c
    pushw	$0
    pushw	$2
    pushw	$msg_entering_protect_mode
    call	print_at_pos
	//Enter protect mode
	//
