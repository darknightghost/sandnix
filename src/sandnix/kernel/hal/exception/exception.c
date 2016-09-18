/*
   Copyright 2016,王思远 <darknightghost.cn@gmail.com>

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

#include "../../../../common/common.h"
#include "../early_print/early_print.h"
#include "../../core/rtl/rtl.h"
#include "../cpu/cpu.h"
#include "../io/io.h"

#include "exception.h"
#include "arch/exception.h"

#define PANIC_BUF_SIZE		1024

static const		char*	errno_tbl[ERRNO_MAX] = {
    [0] = "Succeed",
    [1] = "Operation not permitted",
    [2] = "No such file or directory",
    [3] = "No such process",
    [4] = "Interrupted system call",
    [5] = "I/O error",
    [6] = "No such device or address",
    [7] = "Argument list_t too long",
    [8] = "Exec format error",
    [9] = "Bad file number",
    [10] = "No child processes",
    [11] = "Try again",
    [12] = "Out of memory",
    [13] = "Permission denied",
    [14] = "Bad address",
    [15] = "Block device required",
    [16] = "Device or resource busy",
    [17] = "File Exists",
    [18] = "Cross-device link",
    [19] = "No such device",
    [20] = "Not a directory",
    [21] = "Is a directory",
    [22] = "Invalid argument",
    [23] = "File table overflow",
    [24] = "Too many open files",
    [25] = "Not a typewriter",
    [26] = "Text file busy",
    [27] = "File too large",
    [28] = "No space left on device",
    [29] = "Illegal seek",
    [30] = "Read-only file system",
    [31] = "Too many links",
    [32] = "Broken pipe",
    [33] = "Math argument out of domain of func",
    [34] = "Math result not representable",
    [35] = "Resource deadlock would occur",
    [36] = "File name too long",
    [37] = "No record locks available",
    [38] = "Function not implemented",
    [39] = "Directory not empty",
    [40] = "Too many symbolic links encountered",
    [41] = "Operation would block",
    [42] = "No message of desired type",
    [43] = "Identifier removed",
    [44] = "Channel number out of range",
    [45] = "Level 2 not synchronized",
    [46] = "Level 3 halted",
    [47] = "Level 3 reset",
    [48] = "Link number out of range",
    [49] = "Protocol driver not attached",
    [50] = "No CSI structure available",
    [51] = "Level 2 halted",
    [52] = "Invalid exchange",
    [53] = "Invalid request descriptor",
    [54] = "Exchange full",
    [55] = "No anode",
    [56] = "Invalid request code",
    [57] = "Invalid slot",
    [58] = "Resource deadlock would occur",
    [59] = "Bad font file format",
    [60] = "Device not a stream",
    [61] = "No data available",
    [62] = "Timer expired",
    [63] = "Out of streams resources",
    [64] = "Machine is not on the network",
    [65] = "Package not installed",
    [66] = "Object is remote",
    [67] = "Link has been severed",
    [68] = "Advertise",
    [69] = "Srmount error",
    [70] = "Communication error on send",
    [71] = "Protocol error",
    [72] = "Multihop attempted",
    [73] = "RFS specific error",
    [74] = "Not a data message",
    [75] = "Value too large for defined data type",
    [76] = "Name not unique on network",
    [77] = "File descriptor in bad state",
    [78] = "Remote address changed",
    [79] = "Can not access a needed shared library",
    [80] = "Accessing a corrupted shared library",
    [81] = ".lib section in a.out corrupted",
    [82] = "Attempting to link in too many shared libraries",
    [83] = "Cannot exec a shared library directly",
    [84] = "Illegal byte sequence",
    [85] = "Interrupted system call should be restarted",
    [86] = "Streams pipe",
    [87] = "Too many users",
    [88] = "Socket operation on non-socket",
    [89] = "Destination address required",
    [90] = "Message too long",
    [91] = "Protocol wrong type for socket",
    [92] = "Protocol not available",
    [93] = "Protocol not supported",
    [94] = "Socket type not supported",
    [95] = "Operation not supported on transport endpoint",
    [96] = "Protocol family not supported",
    [97] = "Address family not supported by protocol",
    [98] = "Address already in use",
    [99] = "Cannot assign requested address",
    [100] = "Network is down",
    [101] = "Network is unreachable",
    [102] = "Network dropped connection because of reset",
    [103] = "Software caused connection abort",
    [104] = "Connection reset by peer",
    [105] = "No buffer space available",
    [106] = "Transport endpoint is already connected",
    [107] = "Transport endpoint is not connected",
    [108] = "Cannot send after transport endpoint shutdown",
    [109] = "Too many references: cannot splice",
    [110] = "Connection timed out",
    [111] = "Connection refused",
    [112] = "Host is down",
    [113] = "No route to host",
    [114] = "Operation already in progress",
    [115] = "Operation now in progress",
    [116] = "Stale NFS file handle",
    [117] = "Structure needs cleaning",
    [118] = "Not a XENIX named type file",
    [119] = "No XENIX semaphores available",
    [120] = "Is a named type file",
    [121] = "Remote I/O error",
    [122] = "Quota exceeded",
    [123] = "No medium found",
    [124] = "Wrong medium type",
    [125] = "Operation Canceled",
    [126] = "Required key not available",
    [127] = "Key has expired",
    [128] = "Key has been revoked",
    [129] = "Key was rejected by service",

    //For robust mutexes
    [130] = "Owner died",
    [131] = "State not recoverable",
    [132] = "Operation not possible due to RF-kill",
    [133] = "Memory page has hardware Error",
    [134] = "Not supported",

    //Kernel excepions
    [135] = "Kernel argument error",
    [136] = "Assert",
    [137] = "Unhandled exception interrupt",
    [138] = "Heap corruption",
};

static	char	panic_buf[PANIC_BUF_SIZE];
static	bool	initialized = false;

static	void	lets_die();
static	void	die_ipi_hndlr(pcontext_t p_context, void* p_args);

void hal_exception_init()
{
    hal_early_print_printf("\nInitializing exception module...\n");
    hal_exception_arch_init();

    hal_cpu_regist_IPI_hndlr(IPI_TYPE_DIE, die_ipi_hndlr);
    initialized = true;
}

void hal_exception_panic(char* file, u32 line, u32 error_code, char* fmt, ...)
{
    va_list ap;

    //Disbale interrupts
    hal_io_int_disable();

    //Stop all cpus
    if(initialized) {
        lets_die();
    }

    //TODO:Try to call debugger

    hal_early_print_init();
    hal_early_print_color(FG_BRIGHT_WHITE, BG_RED);
    hal_early_print_cls();
    hal_early_print_puts("\t\t\t>_<||| Sandnix Kernel Paniced\n"
                         "It\'s a pity that an exception has been happend and "
                         "sandnix cannot continue running.The reason of the "
                         "crash is:\n");
    hal_early_print_puts(errno_tbl[error_code]);
    hal_early_print_puts("\n");
    core_rtl_snprintf(panic_buf, PANIC_BUF_SIZE, "File : \"%s\".\nLine : %u.\n",
                      file, line);
    hal_early_print_puts(panic_buf);
    hal_early_print_puts("If you are a developer,it\'s very kind of you to fix "
                         "this problem and push your code to our git repository. "
                         "If you are just a user, you can reboot your computer now.\n");

    va_start(ap, fmt);
    core_rtl_vsnprintf(panic_buf, PANIC_BUF_SIZE, fmt, ap);
    hal_early_print_puts(panic_buf);

    //Stack trace
    die();

    //Never return
    return;
}

void lets_die()
{
    hal_cpu_send_IPI(-1, IPI_TYPE_DIE, NULL);
    return;
}

void die_ipi_hndlr(pcontext_t p_context, void* p_args)
{
    die();
    UNREFERRED_PARAMETER(p_context);
    UNREFERRED_PARAMETER(p_args);
    return;
}
