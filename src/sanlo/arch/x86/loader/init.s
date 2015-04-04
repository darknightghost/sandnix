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
.include	"./segment.inc"
.section	.text 

.global		_start
.global		print_at_pos
//--------------------------------Macros----------------------------------------
.macro		SEGMENT_DESCRIPTOR base,limit,property
	//Limit1 
	.word		(\limit) & 0xFFFF
	//Base1
	.word		(\base) & 0xFFFF
	//Base2
	.byte		((\base) >> 16) & 0xFF
	//Property1+Limit2+property2,G=1
	.word		(((\limit) >> 8) & 0x0F00) | ((\property) & 0xF0FF) | 0x8000
	//Base3
	.byte		((\base) >> 24) & 0xFF
.endm
//------------------------------Definitions-------------------------------------
.equ		LOADER_SIZE,LOADER_SECTORS*512
//Segment descriptor access

.equ		DA_32,0x4000		#32 bit segment

.equ		DA_DPL0,0x00		#DPL = 0
.equ		DA_DPL1,0x20		#DPL = 1
.equ		DA_DPL2,0x40		#DPL = 2
.equ		DA_DPL3,0x60		#DPL = 3

//Data segment
.equ		DA_DR,0x90			#Read-only
.equ		DA_DRW,0x92			#Readable & writeable
.equ		DA_DRWA,0x93		#Readable writeable & accessed

//Code segment
.equ		DA_C,0x98			#Excute-only
.equ		DA_CR,0x9A			#Excuteable & readable
.equ		DA_CCO,0x9C			#Excute-only conforming code segment
.equ		DA_CCOR,0x9E		#Excuteable & readable conforming code segment

.equ		DA_LDT,0x82			#LDT descriptor
.equ		DA_TaskGate,0x85	#Task gate
.equ		DA_386TSS,0x89		#TSS
.equ		DA_386CGate,0x8C	#Call gate
.equ		DA_386IGate,0x8E	#Interrupt gate
.equ		DA_386TGate,0x8F	#Trap gate
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
SEGMENT_DESCRIPTOR	0,				0,					0
descriptor_kernel_data:
SEGMENT_DESCRIPTOR	0,				0xFFFFF,			DA_DRW | DA_DPL0 | DA_32
descriptor_kernel_code:
SEGMENT_DESCRIPTOR	0,				0xFFFFF,			DA_CR | DA_DPL0 | DA_32
descriptor_user_data:
SEGMENT_DESCRIPTOR	0,				0xFFFFF,			DA_DRW | DA_DPL3 | DA_32
descriptor_user_code:
SEGMENT_DESCRIPTOR	0,				0xFFFFF,			DA_CR | DA_DPL3 | DA_32
descriptor_basic_video:
SEGMENT_DESCRIPTOR	0x0B8000,		0xFFFFF,			DA_DRW | DA_DPL0 | DA_32
gdt_end:

gdtr_value:
	.word		(gdt_end - gdt - 1)
	.long		0
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

//void		enable_a20();
//Some part of this function is copied from linux
enable_a20:
		pushw	%bp
		movw	%sp,%bp
		pushw	%bx
		pushw	%cx
		pushw	%dx
		//Try fast A20
		movb	$0x24,%ah
		movb	$1,%al
		int		$0x15
		jnc		_enable_a20_ret
_use_8042:
		cli
		call    wait_input_empty
		movb	$0xAD,%al
		outb	%al,$0x64		#disable Keyboard

		call	wait_input_empty
		movb	$0xD0,%al
		outb	%al,$0x64		#command-read 8042 output port

		call	wait_output_full
		inb		$0x60,%al		#got the value of 8042 output port and save it
		pushw	%ax

		call	wait_input_empty  
		movb	$0xD1,%al
		outb	%al,$0x64		#command-write 8042 output port

		call	wait_input_empty
		popw	%ax
		orb		$0x02,%al		#enable A20 Gate 
		outb	%al,$0x60

		call	wait_input_empty
		movb	$0xAE,%al
		outb	%al,$0x64		#enable Keyboard

		mov		$0x02,%al
		out		%al,$0x92
		sti
_enable_a20_ret:
		popw	%dx
		popw	%cx
		popw	%bx
		movw	%bp,%sp
		popw	%bp
		ret
	
wait_input_empty:
rp1:
		inb		$0x64,%al
		testb	$0x02,%al
		jnz		rp1
		ret

wait_output_full:
rp2:
		inb		$0x64,%al
		testb	$0x01,%al
		jz		rp2
		ret
//--------------------------------Codes-----------------------------------------

_code_start:
		movw	%cs,%ax
		movw	%ax,%ds
		movw	%ax,%es
		pushw	$0x0F
		pushw	$0
		pushw	$0
		pushw	$msg_kernel_loader_loaded
		call	print_at_pos
		addw	$0x08,%sp
		pushw	$0x0F
		pushw	$0
		pushw	$1
		pushw	$msg_entering_protect_mode
		call	print_at_pos
		//Enter protect mode
		//Enable A20 gate
		call	enable_a20
		//Copy gdt to 0x00000000
		cli
		movw	$(gdt_end-gdt),%cx
		movw	$gdt,%si
		movw	$0x0000,%di
		xorw	%ax,%ax
		movw	%ax,%es
		rep		movsb
		//Load gdt
		lgdt	gdtr_value
		//Set CR0.PE
		movl	%cr0,%eax
		orl		$0x00000001,%eax
		movl	%eax,%cr0
		ljmpl	$SELECTOR_K_CODE,$(LOADER_SECTION*0x10+protect_mode_entry)
	
.code32
protect_mode_entry:
		//Initialize segment registers
		movw	$SELECTOR_K_DATA,%ax
		movw	%ax,%ds
		movw	%ax,%ss
		movw	%ax,%es
		movw	%ax,%fs
		movw	$SELECTOR_BASIC_VIDEO,%ax
		movw	%ax,%gs
		//Initialize stack
		movl	$0x00010000,%ebp
		movl	%ebp,%esp
		call	loader_main
		//Reboot
		movb	$0xFE,%al
		outb	%al,$0x64
