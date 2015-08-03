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

#include "../../exceptions.h"
#include "../../../rtl/rtl.h"
#include "../../../setup/setup.h"
#include "../../../io/io.h"
#include "../../../pm/pm.h"

#define	DEFAULT_STDOUT_WIDTH	80
#define	DEFAULT_STDOUT_HEIGHT	25

#define	BG_BLACK			0x00
#define	BG_RED				0x40
#define	BG_WHITE			0x70
#define	FG_BLACK			0x00
#define	FG_BRIGHT_RED		0x0C
#define	FG_BRIGHT_WHITE		0x0F

#define	CRTC_ADDR_REG		0x03D4
#define	CRTC_DATA_REG		0x03D5
#define	CURSOR_POS_H_REG	0x0E
#define	CURSOR_POS_L_REG	0x0F
#define	START_ADDR_H_REG	0x0C
#define	START_ADDR_L_REG	0x0D

static	unsigned short	current_cursor_line = 0;
static	unsigned short	current_cursor_row = 0;

bool	unhndld_excpt_call(u32 int_num, u32 thread_id, u32 err_code);

static	void		print_string(char* str, u8 color, u8 bg_color);
static	void		set_cursor_pos(u16 line, u16 row);
static	void		scroll_down(u16 line, u16 color);
static	void		cls();

static	char		panic_buf[1024];
static	int_hndlr_info	unhndld_info;

static	char*		excpt_tbl[] = {
	"Succeed",
	"Operation not permitted",
	"No such file or directory",
	"No such process",
	"Interrupted system call",
	"I/O error",
	"No such device or address",
	"Argument list too long",
	"Exec format error",
	"Bad file number",
	"No child processes",
	"Try again",
	"Out of memory",
	"Permission denied",
	"Bad address",
	"Block device required",
	"Device or resource busy",
	"File	Exists",
	"Cross-device link",
	"No such device",
	"Not a directory",
	"Is a directory",
	"Invalid argument",
	"File table overflow",
	"Too many open files",
	"Not a typewriter",
	"Text file busy",
	"File too large",
	"No space left on device",
	"Illegal seek",
	"Read-only file system",
	"Too many links",
	"Broken pipe",
	"Math argument out of domain of func",
	"Math result not representable",
	"Resource deadlock would occur",
	"File name too long",
	"No record locks available",
	"Function not implemented",
	"Directory not empty",
	"Too many symbolic links encountered",
	"Operation would block",
	"No message of desired type",
	"Identifier removed",
	"Channel number out of range",
	"Level 2 not synchronized",
	"Level 3 halted",
	"Level 3 reset",
	"Link number out of range",
	"Protocol driver not attached",
	"No CSI structure available",
	"Level 2 halted",
	"Invalid exchange",
	"Invalid request descriptor",
	"Exchange full",
	"No anode",
	"Invalid request code",
	"Invalid slot",
	"Resource deadlock would occur",
	"Bad font file format",
	"Device not a stream",
	"No data available",
	"Timer expired",
	"Out of streams resources",
	"Machine is not on the network",
	"Package not installed",
	"Object is remote",
	"Link has been severed",
	"Advertise	Error",
	"Srmount error",
	"Communication error on send",
	"Protocol error",
	"Multihop attempted",
	"RFS specific error",
	"Not a data message",
	"Value too large for defined data type",
	"Name not unique on network",
	"File descriptor in bad state",
	"Remote address changed",
	"Can not access a needed shared library",
	"Accessing a corrupted shared library",
	".lib section in a.out corrupted",
	"Attempting to link in too many shared libraries",
	"Cannot exec a shared library directly",
	"Illegal byte sequence",
	"Interrupted system call should be restarted",
	"Streams pipe	Error",
	"Too many users",
	"Socket operation on non-socket",
	"Destination address required",
	"Message too long",
	"Protocol wrong type for socket",
	"Protocol not available",
	"Protocol not supported",
	"Socket type not supported",
	"Operation not supported on transport endpoint",
	"Protocol family not supported",
	"Address family not supported by protocol",
	"Address already in use",
	"Cannot assign requested address",
	"Network is down",
	"Network is unreachable",
	"Network dropped connection because of reset",
	"Software caused connection abort",
	"Connection reset by peer",
	"No buffer space available",
	"Transport endpoint is already connected",
	"Transport endpoint is not connected",
	"Cannot send after transport endpoint shutdown",
	"Too many references: cannot splice",
	"Connection timed out",
	"Connection refused",
	"Host is down",
	"No route to host",
	"Operation already in progress",
	"Operation now in progress",
	"Stale NFS file handle",
	"Structure needs cleaning",
	"Not a XENIX named type file",
	"No XENIX semaphores available",
	"Is a named type file",
	"Remote I/O error",
	"Quota exceeded",
	"No medium found",
	"Wrong medium type",
	"Operation Canceled",
	"Required key not available",
	"Key has expired",
	"Key has been revoked",
	"Key was rejected by service",
	"Owner died",
	"State not recoverable",
	"Operation not possible due to RF-kill",
	"Memory page has hardware	Error",
	"Not supported",
	"Kernel argument error"
};

void excpt_init()
{
	u32 i;

	unhndld_info.p_next = NULL;
	unhndld_info.func = unhndld_excpt_call;

	for(i = 0; i <= INT_XF; i++) {
		if(i != INT_RESERVED
		   && i != INT_NMI) {
			io_reg_int_hndlr(i, &unhndld_info);
		}
	}

	return;
}

bool unhndld_excpt_call(u32 int_num, u32 thread_id, u32 err_code)
{
	//context thrd_cntxt;
	u32 cr2;

	//if(pm_get_thread_context(thread_id, &thrd_cntxt)
	// && thrd_cntxt.eip < KERNEL_MEM_BASE) {
	if(0) {
		//Occured in userspace
		//TODO:Kill the process
	} else {
		//Occured in kernel
		switch(int_num) {
		case INT_DE:
			excpt_panic(EFAULT,
			            "Unhandled #DE,thread id is %u,\nError code is %p.\n",
			            thread_id,
			            err_code);
			break;

		case INT_DB:
			excpt_panic(EFAULT,
			            "Unhandled #DB,thread id is %u,\nError code is %p.\n",
			            thread_id,
			            err_code);
			break;

		case INT_BP:
			excpt_panic(EFAULT,
			            "Unhandled #BP,thread id is %u,\nError code is %p.\n",
			            thread_id,
			            err_code);
			break;

		case INT_OF:
			excpt_panic(EFAULT,
			            "Unhandled #OF,thread id is %u,\nError code is %p.\n",
			            thread_id,
			            err_code);
			break;

		case INT_BR:
			excpt_panic(EFAULT,
			            "Unhandled #BR,thread id is %u,\nError code is %p.\n",
			            thread_id,
			            err_code);
			break;

		case INT_UD:
			excpt_panic(EFAULT,
			            "Unhandled #UD,thread id is %u,\nError code is %p.\n",
			            thread_id,
			            err_code);
			break;

		case INT_NM:
			excpt_panic(EFAULT,
			            "Unhandled #NM,thread id is %u,\nError code is %p.\n",
			            thread_id,
			            err_code);
			break;

		case INT_DF:
			excpt_panic(EFAULT,
			            "Unhandled #DF,thread id is %u,\nError code is %p.\n",
			            thread_id,
			            err_code);
			break;

		case INT_FPU:
			excpt_panic(EFAULT,
			            "Unhandled #FPU,thread id is %u,\nError code is %p.\n",
			            thread_id,
			            err_code);
			break;

		case INT_TS:
			excpt_panic(EFAULT,
			            "Unhandled #TS,thread id is %u,\nError code is %p.\n",
			            thread_id,
			            err_code);
			break;

		case INT_NP:
			excpt_panic(EFAULT,
			            "Unhandled #NP,thread id is %u,\nError code is %p.\n",
			            thread_id,
			            err_code);
			break;

		case INT_SS:
			excpt_panic(EFAULT,
			            "Unhandled #SS,thread id is %u,\nError code is %p.\n",
			            thread_id,
			            err_code);
			break;

		case INT_GP:
			excpt_panic(EFAULT,
			            "Unhandled #GP,thread id is %u,\nError code is %p.\n",
			            thread_id,
			            err_code);
			break;

		case INT_PF:
			__asm__ __volatile__(
			    "movl	%%cr2,%0\n\t"
			    :"=r"(cr2)
			    :);
			excpt_panic(EFAULT,
			            "Unhandled #PF,thread id is %u,\nError code is %p.\nCR2 = %p\n",
			            thread_id,
			            err_code,
			            cr2);
			break;

		case INT_MF:
			excpt_panic(EFAULT,
			            "Unhandled #MF,thread id is %u,\nError code is %p.\n",
			            thread_id,
			            err_code);
			break;

		case INT_AC:
			excpt_panic(EFAULT,
			            "Unhandled #AC,thread id is %u,\nError code is %p.\n",
			            thread_id,
			            err_code);
			break;

		case INT_MC:
			excpt_panic(EFAULT,
			            "Unhandled #MC,thread id is %u,\nError code is %p.\n",
			            thread_id,
			            err_code);
			break;

		case INT_XF:
			excpt_panic(EFAULT,
			            "Unhandled #XF,thread id is %u,\nError code is %p.\n",
			            thread_id,
			            err_code);
			break;
		}

		return true;
	}

	return true;
}

void excpt_panic(u32 reason, char* fmt, ...)
{
	va_list args;

	//Disable interrupt
	__asm__ __volatile__(
	    "cli\n\t"
	);

	//Reset video card to text mode

	//Clear screen
	cls();

	//Print infomations of exceptions
	print_string(">_<|||\nIt\'s a pity that an exception has been happend and sandnix cannot continue running.The reason of the crash is:\n\n", FG_BRIGHT_WHITE, BG_RED);
	print_string(excpt_tbl[reason], FG_BRIGHT_WHITE, BG_RED);
	print_string("\n\n", FG_BRIGHT_WHITE, BG_RED);
	va_start(args, fmt);
	rtl_vprintf_s(panic_buf, 1024, fmt, args);
	va_end(args);
	print_string(panic_buf, FG_BRIGHT_WHITE, BG_RED);

	while(1);

	//Never return,in fact.
	return;
}

void cls()
{
	size_t size;
	size = DEFAULT_STDOUT_WIDTH * DEFAULT_STDOUT_HEIGHT;
	__asm__ __volatile__(
	    "cld\n\t"
	    "movl		%2,%%edi\n\t"
	    "movl		%0,%%ecx\n\t"
	    "movb		%1,%%ah\n\t"
	    "movb		$0x20,%%al\n\t"
	    "rep		stosw"
	    ::"m"(size), "i"(FG_BRIGHT_WHITE | BG_RED), "i"(BASIC_VIDEO_BASE_ADDR));
	current_cursor_line = 0;
	current_cursor_row = 0;
	set_cursor_pos(0, 0);
	return;
}


void print_string(char* str, u8 color, u8 bg_color)
{
	char* p;
	u16 character;
	u16 offset;
	u16* p_video_mem;

	for(p = str; *p != '\0'; p++) {
		if(*p == '\n') {
			current_cursor_line++;
			current_cursor_row = 0;

			if(current_cursor_line >= DEFAULT_STDOUT_HEIGHT) {
				//Scroll down
				current_cursor_line--;
				scroll_down(1, bg_color);
			}

			set_cursor_pos(
			    current_cursor_line,
			    current_cursor_row);

		} else if(*p == '\t') {
			if(current_cursor_row % 4 == 0) {
				print_string(" ", color, bg_color);
			}

			while(current_cursor_row % 4 != 0) {
				print_string(" ", color, bg_color);
			}

		} else {
			//Print character
			character = (u16)(color | bg_color) * 0x100 + *p;
			offset =
			    (current_cursor_line * DEFAULT_STDOUT_WIDTH
			     + current_cursor_row)
			    * 2;
			p_video_mem = (u16*)((u8*)BASIC_VIDEO_BASE_ADDR + offset);
			*p_video_mem = character;
			current_cursor_row++;

			if(current_cursor_row >= DEFAULT_STDOUT_WIDTH) {
				current_cursor_row = 0;
				current_cursor_line++;

				if(current_cursor_line >= DEFAULT_STDOUT_HEIGHT) {
					//Scroll down
					current_cursor_line--;
					scroll_down(1, bg_color);
				}
			}

			set_cursor_pos(
			    current_cursor_line,
			    current_cursor_row);
		}
	}

	return;
}

void set_cursor_pos(u16 line, u16 row)
{
	u16 pos;

	//Check the range of position
	if(line >= DEFAULT_STDOUT_HEIGHT
	   || row >= DEFAULT_STDOUT_WIDTH) {
		return;
	}

	pos = line * DEFAULT_STDOUT_WIDTH + row;
	current_cursor_line = line;
	current_cursor_row = row;
	//Disable interruptions
	__asm__ __volatile__(
	    "pushf\n\t"
	    "cli\n\t");
	io_write_port_byte((u8)CURSOR_POS_H_REG, (u16)CRTC_ADDR_REG);
	io_write_port_byte((u8)((pos >> 8) & 0xFF), (u16)CRTC_DATA_REG);
	io_write_port_byte((u8)CURSOR_POS_L_REG, (u16)CRTC_ADDR_REG);
	io_write_port_byte((u8)(pos & 0xFF), (u16)CRTC_DATA_REG);
	__asm__ __volatile__(
	    "popf\n\t");
	return;
}

void scroll_down(u16 line, u16 color)
{
	u16 offset;
	u16 len;
	u16	half_len;

	if(line >= DEFAULT_STDOUT_HEIGHT) {
		cls(color);
		return;
	}

	offset = line * DEFAULT_STDOUT_WIDTH * 2;
	len = DEFAULT_STDOUT_HEIGHT * DEFAULT_STDOUT_WIDTH * 2 - offset;
	half_len = offset / 2;
	__asm__ __volatile__(
	    "cld\n\t"
	    "push		%%es\n\t"
	    "push		%%ds\n\t"
	    "movw		%%gs,%%ax\n\t"
	    "movw		%%ax,%%es\n\t"
	    "movw		%%ax,%%ds\n\t"
	    "movzwl		%0,%%esi\n\t"
	    "xorl		%%edi,%%edi\n\t"
	    "movzwl		%1,%%ecx\n\t"
	    "rep		movsb\n\t"
	    "movzwl		%1,%%edi\n\t"
	    "movzwl		%3,%%ecx\n\t"
	    "movb		%2,%%ah\n\t"
	    "movb		$0x20,%%al\n\t"
	    "rep		stosw\n\t"
	    "pop		%%ds\n\t"
	    "pop		%%es\n\t"
	    ::"m"(offset), "m"(len), "m"(color), "m"(half_len));
	return;
}
