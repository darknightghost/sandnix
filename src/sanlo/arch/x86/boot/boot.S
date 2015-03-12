.section	.text 
.global		_start
//------------------------------Definitions-------------------------------------
//--------------------------------Entery----------------------------------------
.code16
_start:  
    movw	%cs,%ax
    movw	%ax,%ds
    movw	%ax,%es
    call	cls
    pushw	$0x0c
    pushw	$0
    pushw	$0
    pushw	$msg_search_config
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

//Hard disk functions
//bool		read_disk(unsigned short disk,)
read_disk:
	pushw	%bp
	movw	%sp,%bp
	movw	%bp,%sp
	popw	%bp
	ret

//void*		get_partion_offset(unsigned short disk,unsigned short partion);
get_partion_offset:
	pushw	%bp
	movw	%sp,%bp
	movw	%bp,%sp
	popw	%bp
	ret

msg_search_config:  
    .asciz  "Searching for kernel loader..."
screen_line:
	.byte	0
screen_row:
	.byte	0
.org		0x1fe,0x90
.word		0xaa55  
