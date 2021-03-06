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

#pragma once

#include "../../../../../common/common.h"
#include "./process_defs.h"
#include "../../rtl/obj/obj_defs.h"
#include "../../rtl/container/map/map_defs.h"
#include "../../rtl/container/list/list_defs.h"
#include "../lock/cond/cond_defs.h"
#include "../lock/mutex/mutex_defs.h"

typedef	struct	_process_obj {
    obj_t					obj;			//Base object
    u32						process_id;		//Process id
    struct _process_obj*	p_parent;		//Parent process object
    u32						status;			//Process status
    u32						exit_code;		//Exit code
    pkstring_obj_t			cmd_line;		//Process command line
    pmutex_t				p_tbl_lock;		//Process table lock

    //Subsystem
    u32			subsys;				//Sub system

    //Authority
    u32			ruid;				//Real user id
    u32			euid;				//Effective user id
    u32			suid;				//Saved set-user-id
    u32			rgid;				//Real group id
    u32			egid;				//Effective group id
    u32			sgid;				//Saved set-group-id
    list_t		groups;				//Other groups

    //Environment
    map_t		env_vars;			//Environment variables

    //Threads
    u32			alive_thread_num;	//How many threads does the process have
    map_t		alive_threads;		//Alive threads
    map_t		zombie_threads;		//Zombie threads
    cond_t		thrd_wait_cond;
    list_t		thrd_wait_list;

    //Referenced objects
    map_t		ref_objs;			//Referenced objects

    //Child processes
    map_t		alive_children;		//Alive child processes
    map_t		zombie_children;	//Zombie child processes
    cond_t		child_wait_cond;
    list_t		child_wait_list;

    //Methods
    //pprocess_obj_t		fork(pprocess_obj_t p_this, u32 new_process_id);
    struct _process_obj*	(*fork)(struct _process_obj*, u32);

    //void					add_ref_obj(pprocess_obj_t p_this, pproc_ref_obj_t p_ref_obj);
    void	(*add_ref_obj)(struct _process_obj*, pproc_ref_obj_t);

    //void					die(pprocess_obj_t p_this);
    void	(*die)(struct _process_obj*);

    //void	add_thread(pprocess_obj_t p_this, u32 thread_id);
    void	(*add_thread)(struct _process_obj*, u32);

    //void	zombie_thread(pprocess_obj_t p_this, u32 thread_id);
    void	(*zombie_thread)(struct _process_obj*, u32);

    //void	remove_thread(pprocess_obj_t p_this, u32 thread_id);
    void	(*remove_thread)(struct _process_obj*, u32);

    //kstatus_t	wait_for_zombie_thread(pprocess_obj_t p_this, bool by_id,
    //	u32* p_thread_id);
    kstatus_t	(*wait_for_zombie_thread)(struct _process_obj*, bool, u32*);

    //kstatus_t	wait_for_zombie_child(pprocess_obj_t p_this, bool by_id,
    //	u32* p_zombie_child_id);
    kstatus_t	(*wait_for_zombie_child)(struct _process_obj*, bool, u32*,
                                         struct _process_obj**);

} process_obj_t, *pprocess_obj_t;

typedef	struct	_proc_ref_proc_t {
    pprocess_obj_t	p_process;
    bool			waited;
    cond_t			cond;
    u32				ref;
} proc_child_info_t, *pproc_child_info_t;

typedef	struct	_proc_ref_thrd_t {
    u32			id;
    bool		waited;
    cond_t		cond;
    u32			ref;
} proc_thrd_info_t, *pproc_thrd_info_t;

typedef	struct	_thrd_wait_info {
    u32			id;
} thrd_wait_info_t, *pthrd_wait_info_t;

typedef	struct	_proc_wait_info {
    pprocess_obj_t	p_proc_obj;
} proc_wait_info_t, *pproc_wait_info_t;
