.section	.text 
.global		_start
//------------------------------Definitions-------------------------------------
.equ		DAP_SIZE,0x10
.equ		DAP_RESERVED1_OFFSET,0x01
.equ		DAP_SECTOR_NUM_OFFSET,0x02
.equ		DAP_RESERVED2_OFFSET,0x03
.equ		DAP_BUF_ADDRESS_OFFSET,0x04
.equ		DAP_LBA_ADDRESS_OFFSET,0x08
//--------------------------------BPB----------------------------------------
.code16
_start:
	jmp		_code_start
	nop

//BPB
BS_OEMNAME:					
	.ascii	"SANDNIX\0"				#Must be 8 bytes
BPB_BytePerSec:
	.word		512
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
msg_search_kernel_loader:
    .asciz  "Loading kernel loader..."
    
//--------------------------------Codes-----------------------------------------
_code_start:
    movw	%cs,%ax
    movw	%ax,%ds
    movw	%ax,%es
    call	cls
    pushw	$0x0c
    pushw	$0
    pushw	$0
    pushw	$msg_search_kernel_loader
    call	print_at_pos
_bak:
	jmp		_bak

//------------------------------Functions--------------------------------------

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

.org		0x1fe,0x90
.word		0xaa55  
