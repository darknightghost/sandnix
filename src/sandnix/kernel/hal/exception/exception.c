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

#include "../../core/rtl/rtl.h"
#include "../../core/pm/pm.h"

#include "../early_print/early_print.h"
#include "../cpu/cpu.h"
#include "../io/io.h"

#include "arch/exception.h"

#include "exception.h"

#define PANIC_BUF_SIZE		1024

static const		char*	errno_tbl[ERRNO_MAX + 1] = {
    //User exceptions
    [ESUCCESS] = "Succeed",
    [EPERM] = "Operation not permitted",
    [ENOENT] = "No such file or directory",
    [ESRCH] = "No such process",
    [EINTR] = "Interrupted system call",
    [EIO] = "I/O error",
    [ENXIO] = "No such device or address",
    [E2BIG] = "Argument list_t too long",
    [ENOEXEC] = "Exec format error",
    [EBADF] = "Bad file number",
    [ECHILD] = "No child processes",
    [EAGAIN] = "Try again",
    [ENOMEM] = "Out of memory",
    [EACCES] = "Permission denied",
    [EFAULT] = "Bad address",
    [ENOTBLK] = "Block device required",
    [EBUSY] = "Device or resource busy",
    [EEXIST] = "File Exists",
    [EXDEV] = "Cross-device link",
    [ENODEV] = "No such device",
    [ENOTDIR] = "Not a directory",
    [EISDIR] = "Is a directory",
    [EINVAL] = "Invalid argument",
    [ENFILE] = "File table overflow",
    [EMFILE] = "Too many open files",
    [ENOTTY] = "Not a typewriter",
    [ETXTBSY] = "Text file busy",
    [EFBIG] = "File too large",
    [ENOSPC] = "No space left on device",
    [ESPIPE] = "Illegal seek",
    [EROFS] = "Read-only file system",
    [EMLINK] = "Too many links",
    [EPIPE] = "Broken pipe",
    [EDOM] = "Math argument out of domain of func",
    [ERANGE] = "Math result not representable",
    [EDEADLK] = "Resource deadlock would occur",
    [ENAMETOOLONG] = "File name too long",
    [ENOLCK] = "No record locks available",
    [ENOSYS] = "Function not implemented",
    [ENOTEMPTY] = "Directory not empty",
    [ELOOP] = "Too many symbolic links encountered",
    [EWOULDBLOCK] = "Operation would block",
    [ENOMSG] = "No message of desired type",
    [EIDRM] = "Identifier removed",
    [ECHRNG] = "Channel number out of range",
    [EL2NSYNC] = "Level 2 not synchronized",
    [EL3HLT] = "Level 3 halted",
    [EL3RST] = "Level 3 reset",
    [ELNRNG] = "Link number out of range",
    [EUNATCH] = "Protocol driver not attached",
    [ENOCSI] = "No CSI structure available",
    [EL2HLT] = "Level 2 halted",
    [EBADE] = "Invalid exchange",
    [EBADR] = "Invalid request descriptor",
    [EXFULL] = "Exchange full",
    [ENOANO] = "No anode",
    [EBADRQC] = "Invalid request code",
    [EBADSLT] = "Invalid slot",
    [EDEADLOCK] = "Resource deadlock would occur",
    [EBFONT] = "Bad font file format",
    [ENOSTR] = "Device not a stream",
    [ENODATA] = "No data available",
    [ETIME] = "Timer expired",
    [ENOSR] = "Out of streams resources",
    [ENONET] = "Machine is not on the network",
    [ENOPKG] = "Package not installed",
    [EREMOTE] = "Object is remote",
    [ENOLINK] = "Link has been severed",
    [EADV] = "Advertise Error",
    [ESRMNT] = "Srmount error",
    [ECOMM] = "Communication error on send",
    [EPROTO] = "Protocol error",
    [EMULTIHOP] = "Multihop attempted",
    [EDOTDOT] = "RFS specific error",
    [EBADMSG] = "Not a data message",
    [EOVERFLOW] = "Value too large for defined data type",
    [ENOTUNIQ] = "Name not unique on network",
    [EBADFD] = "File descriptor in bad state",
    [EREMCHG] = "Remote address changed",
    [ELIBACC] = "Can not access a needed shared library",
    [ELIBBAD] = "Accessing a corrupted shared library",
    [ELIBSCN] = ".lib section in a.out corrupted",
    [ELIBMAX] = "Attempting to link in too many shared libraries",
    [ELIBEXEC] = "Cannot exec a shared library directly",
    [EILSEQ] = "Illegal byte sequence",
    [ERESTART] = "Interrupted system call should be restarted",
    [ESTRPIPE] = "Streams pipe Error",
    [EUSERS] = "Too many users",
    [ENOTSOCK] = "Socket operation on non-socket",
    [EDESTADDRREQ] = "Destination address required",
    [EMSGSIZE] = "Message too long",
    [EPROTOTYPE] = "Protocol wrong type for socket",
    [ENOPROTOOPT] = "Protocol not available",
    [EPROTONOSUPPORT] = "Protocol not supported",
    [ESOCKTNOSUPPORT] = "Socket type not supported",
    [EOPNOTSUPP] = "Operation not supported on transport endpoint",
    [EPFNOSUPPORT] = "Protocol family not supported",
    [EAFNOSUPPORT] = "Address family not supported by protocol",
    [EADDRINUSE] = "Address already in use",
    [EADDRNOTAVAIL] = "Cannot assign requested address",
    [ENETDOWN] = "Network is down",
    [ENETUNREACH] = "Network is unreachable",
    [ENETRESET] = "Network dropped connection because of reset",
    [ECONNABORTED] = "Software caused connection abort",
    [ECONNRESET] = "Connection reset by peer",
    [ENOBUFS] = "No buffer space available",
    [EISCONN] = "Transport endpoint is already connected",
    [ENOTCONN] = "Transport endpoint is not connected",
    [ESHUTDOWN] = "Cannot send after transport endpoint shutdown",
    [ETOOMANYREFS] = "Too many references: cannot splice",
    [ETIMEDOUT] = "Connection timed out",
    [ECONNREFUSED] = "Connection refused",
    [EHOSTDOWN] = "Host is down",
    [EHOSTUNREACH] = "No route to host",
    [EALREADY] = "Operation already in progress",
    [EINPROGRESS] = "Operation now in progress",
    [ESTALE] = "Stale NFS file handle",
    [EUCLEAN] = "Structure needs cleaning",
    [ENOTNAM] = "Not a XENIX named type file",
    [ENAVAIL] = "No XENIX semaphores available",
    [EISNAM] = "Is a named type file",
    [EREMOTEIO] = "Remote I/O error",
    [EDQUOT] = "Quota exceeded",
    [ENOMEDIUM] = "No medium found",
    [EMEDIUMTYPE] = "Wrong medium type",
    [ECANCELED] = "Operation Canceled",
    [ENOKEY] = "Required key not available",
    [EKEYEXPIRED] = "Key has expired",
    [EKEYREVOKED] = "Key has been revoked",
    [EKEYREJECTED] = "Key was rejected by service",

    //For robust mutexes
    [EOWNERDEAD] = "Owner died",
    [ENOTRECOVERABLE] = "State not recoverable",
    [ERFKILL] = "Operation not possible due to RF-kill",
    [EHWPOISON] = "Memory page has hardware Error",
    [ENOTSUP] = "Not supported",

    //Sandnix kernel exceptions
    //Debugging
    [EASSERT] = "Assert",
    [EBREAKPOINT] = "Breakpoint",

    //Kernel arguments
    [EKERNELARG] = "Kernel argument error",

    //Memory
    [EHPCORRUPTION] = "Heap corruption",
    [EPAGEREAD] = "Page cannot be read",
    [EPAGEWRITE] = "Page cannot be write",
    [EPAGEEXEC] = "Page cannot be executed",
    [EPFINPAGING] = "Page fault in paging module",

    //Math
    [EDIV] = "Divide error",
    [EFLOAT] = "Math coprosessor error",

    //Opcode
    [EUNDEFINED] = "Undefined opcode",

    //Privilege
    [EPRIVILEGE] = "Privilege error",

    //Others
    [EIRETVAL] = "Illegal return value",
    [EUNKNOWINT] = "Unknow interrupt"
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

const char* hal_exception_get_err_name(kstatus_t error_code)
{
    return errno_tbl[error_code];
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
