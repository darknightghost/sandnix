/*
    Copyright 2017,王思远 <darknightghost.cn@gmail.com>

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

#include "../../../../../common/common.h"
#include "../../../hal/mmu/mmu.h"
#include "../../rtl/rtl.h"
#include "../../mm/mm.h"

#include "./signal.h"
#include "./thrd_signal_tbl_obj.h"

#define	MODULE_NAME		core_ipc

static	pheap_t		signal_obj_heap = NULL;


static	void					destructor(pthrd_signal_tbl_obj_t p_this);
static	int						compare(pthrd_signal_tbl_obj_t p_this,
                                        pthrd_signal_tbl_obj_t p_1);
static	pkstring_obj_t			to_string(pthrd_signal_tbl_obj_t p_this);
static	pthrd_signal_tbl_obj_t	on_fork(pthrd_signal_tbl_obj_t p_this,
                                        u32 process_id);
static	void			signal(u32 sig);
static	void			set_default(u32 sig);
static	void			set_attr(u32 sig, u32 attr);
static	u32				get_attr(u32 sig);
static	sig_handler		set_hndlr(u32 sig, sig_handler hndlr);
static	void			do_signal(u32 sig);

pthrd_signal_tbl_obj_t thrd_signal_tbl_obj(u32 thread_id, u32 process_id)
{
    if(signal_obj_heap == NULL) {
        signal_obj_heap = core_mm_heap_create(
                              HEAP_MULITHREAD,
                              SANDNIX_KERNEL_PAGE_SIZE);
    }

    //Allocate memory
    pthrd_signal_tbl_obj_t p_ret = NULL;

    while(p_ret == NULL) {
        p_ret = (pthrd_signal_tbl_obj_t)thread_ref_obj(
                    thread_id,
                    process_id,
                    CLASS_THREAD_SIGNAL_TBL_OBJ,
                    (thread_obj_fork_t)on_fork,
                    (destructor_t)destructor,
                    (compare_obj_t)compare,
                    (to_string_t)to_string,
                    signal_obj_heap,
                    sizeof(thrd_signal_tbl_obj));
    }

    //Set value
    p_ret->cur_signal = -1;
    core_rtl_memset(&(p_ret->status), 0, sizeof(p_ret->status));

    //Initialize signals
    //SIGHUP
    INIT_SIGNAL_STAT(
        p_ret->status[SIGHUP],
        PRIVATE(signal_quit),
        0,
        SIG_MASK_CANBECAUGHT | SIG_MASK_CANBEIGNORE);

    //SIGINT
    INIT_SIGNAL_STAT(
        p_ret->status[SIGINT],
        PRIVATE(signal_quit),
        0,
        SIG_MASK_CANBECAUGHT | SIG_MASK_CANBEIGNORE);

    //SIGQUIT
    INIT_SIGNAL_STAT(
        p_ret->status[SIGQUIT],
        PRIVATE(signal_quit_with_core_dump),
        0,
        SIG_MASK_CANBECAUGHT | SIG_MASK_CANBEIGNORE);

    //SIGILL
    INIT_SIGNAL_STAT(
        p_ret->status[SIGILL],
        PRIVATE(signal_quit_with_core_dump),
        0,
        SIG_MASK_CANBECAUGHT | SIG_MASK_CANBEIGNORE);

    //SIGTRAP
    INIT_SIGNAL_STAT(
        p_ret->status[SIGTRAP],
        PRIVATE(signal_quit_with_core_dump),
        0,
        SIG_MASK_CANBECAUGHT | SIG_MASK_CANBEIGNORE);

    //SIGABRT
    INIT_SIGNAL_STAT(
        p_ret->status[SIGABRT],
        PRIVATE(signal_quit_with_core_dump),
        0,
        SIG_MASK_CANBECAUGHT | SIG_MASK_CANBEIGNORE);

    //SIGIOT
    INIT_SIGNAL_STAT(
        p_ret->status[SIGIOT],
        PRIVATE(signal_quit_with_core_dump),
        0,
        SIG_MASK_CANBECAUGHT | SIG_MASK_CANBEIGNORE);

    //SIGBUS
    INIT_SIGNAL_STAT(
        p_ret->status[SIGBUS],
        PRIVATE(signal_quit_with_core_dump),
        0,
        SIG_MASK_CANBECAUGHT | SIG_MASK_CANBEIGNORE);

    //SIGFPE
    INIT_SIGNAL_STAT(
        p_ret->status[SIGFPE],
        PRIVATE(signal_quit_with_core_dump),
        0,
        SIG_MASK_CANBECAUGHT | SIG_MASK_CANBEIGNORE);

    //SIGKILL
    INIT_SIGNAL_STAT(
        p_ret->status[SIGKILL],
        PRIVATE(signal_quit),
        0,
        0);

    //SIGUSR1
    INIT_SIGNAL_STAT(
        p_ret->status[SIGUSR1],
        PRIVATE(signal_quit),
        0,
        SIG_MASK_CANBECAUGHT | SIG_MASK_CANBEIGNORE);

    //SIGSEGV
    INIT_SIGNAL_STAT(
        p_ret->status[SIGSEGV],
        PRIVATE(signal_quit_with_core_dump),
        0,
        SIG_MASK_CANBECAUGHT | SIG_MASK_CANBEIGNORE);

    //SIGUSR2
    INIT_SIGNAL_STAT(
        p_ret->status[SIGUSR2],
        PRIVATE(signal_quit),
        0,
        SIG_MASK_CANBECAUGHT | SIG_MASK_CANBEIGNORE);

    //SIGPIPE
    INIT_SIGNAL_STAT(
        p_ret->status[SIGPIPE],
        PRIVATE(signal_quit),
        0,
        SIG_MASK_CANBECAUGHT | SIG_MASK_CANBEIGNORE);

    //SIGALRM
    INIT_SIGNAL_STAT(
        p_ret->status[SIGALRM],
        PRIVATE(signal_quit),
        0,
        SIG_MASK_CANBECAUGHT | SIG_MASK_CANBEIGNORE);

    //SIGTERM
    INIT_SIGNAL_STAT(
        p_ret->status[SIGTERM],
        PRIVATE(signal_quit),
        0,
        SIG_MASK_CANBECAUGHT | SIG_MASK_CANBEIGNORE);

    //SIGSTKFLT
    INIT_SIGNAL_STAT(
        p_ret->status[SIGSTKFLT],
        PRIVATE(signal_quit),
        0,
        SIG_MASK_CANBECAUGHT | SIG_MASK_CANBEIGNORE);

    //SIGCHLD
    INIT_SIGNAL_STAT(
        p_ret->status[SIGCHLD],
        NULL,
        SIG_ATTR_IGNORE,
        SIG_MASK_CANBECAUGHT | SIG_MASK_CANBEIGNORE);

    //SIGCONT
    INIT_SIGNAL_STAT(
        p_ret->status[SIGCONT],
        PRIVATE(signal_cont),
        0,
        SIG_MASK_CANBECAUGHT | SIG_MASK_CANBEIGNORE);

    //SIGSTOP
    INIT_SIGNAL_STAT(
        p_ret->status[SIGSTOP],
        PRIVATE(signal_stop),
        0,
        0);

    //SIGTSTP
    INIT_SIGNAL_STAT(
        p_ret->status[SIGTSTP],
        PRIVATE(signal_stop),
        0,
        SIG_MASK_CANBECAUGHT | SIG_MASK_CANBEIGNORE);

    //SIGTTIN
    INIT_SIGNAL_STAT(
        p_ret->status[SIGTTIN],
        PRIVATE(signal_stop),
        0,
        SIG_MASK_CANBECAUGHT | SIG_MASK_CANBEIGNORE);

    //SIGTTOU
    INIT_SIGNAL_STAT(
        p_ret->status[SIGTTOU],
        PRIVATE(signal_stop),
        0,
        SIG_MASK_CANBECAUGHT | SIG_MASK_CANBEIGNORE);

    //SIGURG
    INIT_SIGNAL_STAT(
        p_ret->status[SIGURG],
        NULL,
        SIG_ATTR_IGNORE,
        SIG_MASK_CANBECAUGHT | SIG_MASK_CANBEIGNORE);

    //SIGXCPU
    INIT_SIGNAL_STAT(
        p_ret->status[SIGXCPU],
        PRIVATE(signal_quit_with_core_dump),
        0,
        SIG_MASK_CANBECAUGHT | SIG_MASK_CANBEIGNORE);

    //SIGXFSZ
    INIT_SIGNAL_STAT(
        p_ret->status[SIGXFSZ],
        PRIVATE(signal_quit_with_core_dump),
        0,
        SIG_MASK_CANBECAUGHT | SIG_MASK_CANBEIGNORE);

    //SIGVTALRM
    INIT_SIGNAL_STAT(
        p_ret->status[SIGVTALRM],
        PRIVATE(signal_quit),
        0,
        SIG_MASK_CANBECAUGHT | SIG_MASK_CANBEIGNORE);

    //SIGPROF
    INIT_SIGNAL_STAT(
        p_ret->status[SIGPROF],
        PRIVATE(signal_quit),
        0,
        SIG_MASK_CANBECAUGHT | SIG_MASK_CANBEIGNORE);

    //SIGWINCH
    INIT_SIGNAL_STAT(
        p_ret->status[SIGWINCH],
        NULL,
        SIG_ATTR_IGNORE,
        SIG_MASK_CANBECAUGHT | SIG_MASK_CANBEIGNORE);

    //SIGIO
    INIT_SIGNAL_STAT(
        p_ret->status[SIGIO],
        PRIVATE(signal_quit),
        0,
        SIG_MASK_CANBECAUGHT | SIG_MASK_CANBEIGNORE);

    //SIGPWR
    INIT_SIGNAL_STAT(
        p_ret->status[SIGPWR],
        PRIVATE(signal_quit),
        0,
        SIG_MASK_CANBECAUGHT | SIG_MASK_CANBEIGNORE);

    //SIGSYS
    INIT_SIGNAL_STAT(
        p_ret->status[SIGSYS],
        PRIVATE(signal_quit),
        0,
        SIG_MASK_CANBECAUGHT | SIG_MASK_CANBEIGNORE);

    //Methods
    p_ret->signal = signal;
    p_ret->set_default = set_default;
    p_ret->set_attr = set_attr;
    p_ret->get_attr = get_attr;
    p_ret->set_hndlr = set_hndlr;
    p_ret->do_signal = do_signal;

    return p_ret;
}

void					destructor(pthrd_signal_tbl_obj_t p_this);
int						compare(pthrd_signal_tbl_obj_t p_this,
                                pthrd_signal_tbl_obj_t p_1);
pkstring_obj_t			to_string(pthrd_signal_tbl_obj_t p_this);
pthrd_signal_tbl_obj_t	on_fork(pthrd_signal_tbl_obj_t p_this,
                                u32 thread_id);
void			signal(u32 sig);
void			set_default(u32 sig);
void			set_attr(u32 sig, u32 attr);
u32				get_attr(u32 sig);
sig_handler		set_hndlr(u32 sig, sig_handler hndlr);
void			do_signal(u32 sig);
