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

#pragma once

#include "../../../../../common/common.h"
#include "signal_defs.h"
#include "../../pm/pm.h"

//Signal attributes
#define	SIG_ATTR_IGNORE			0x01
#define	SIG_ATTR_CAUGHT			0x02
#define	SIG_ATTR_DEADLY			0x04

//Masks
#define	SIG_MASK_CANBEIGNORE	SIG_ATTR_IGNORE
#define	SIG_MASK_CANBECAUGHT	SIG_ATTR_CAUGHT

typedef	struct	_signal_status {
    sig_handler		handler;			//Signal handler, must in user space
    sig_handler		default_behavior;	//Default handler, must in kernel space
    bool			is_handling;		//If the signal is being handled.
    u32				count;				//Count
    u32				attr;				//Attrubute
    u32				default_attr;		//Default attrubute
    u32				attr_set_mask;		//Bits can be set
} signal_status_t, *psignal_status_t;

#define	INIT_SIGNAL_STAT(stat_struct, hndlr, attributes, mask) { \
        (stat_struct).handler = NULL; \
        (stat_struct).default_behavior = (hndlr); \
        (stat_struct).is_handling = false; \
        (stat_struct).count = 0;\
        (stat_struct).attr = (attributes); \
        (stat_struct).default_attr = (attributes); \
        (stat_struct).attr_set_mask = (mask); \
    }

typedef	struct _thrd_signal_tbl_obj {
    thread_ref_obj_t	parent;			//Parent class
    //If a signal is handling, cur_signal is the handling signal, otherwise it is -1.
    s32					cur_signal;
    signal_status_t		status[_NSIG];	//Signal status

    //Methods
    //void	signal(u32 sig);
    void	(*signal)(u32 sig);

    //void	set_default(u32 sig);
    void	(*set_default)(u32);

    //void	set_attr(u32 sig, u32 attr);
    void	(*set_attr)(u32, u32);

    //u32	get_attr(u32 sig);
    u32(*get_attr)(u32);

    //sig_handler	set_hndlr(u32 sig, sig_handler hndlr);
    sig_handler(*set_hndlr)(u32, sig_handler);

    //void	do_signal(u32 sig);
    void	(*do_signal)(u32);

} thrd_signal_tbl_obj_t, *pthrd_signal_tbl_obj_t;


