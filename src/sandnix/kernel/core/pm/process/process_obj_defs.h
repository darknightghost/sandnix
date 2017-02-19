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

typedef	struct	_process_obj {
    obj_t		obj;				//Base object
    u32			process_id;			//Process id
    u32			parent_id;			//Parent process id
    u32			exit_code;			//Exit code

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

    //Referenced objects
    map_t		ref_objs;			//Referenced objects

    //Child processes
    map_t		alive_children;		//Alive child processes
    map_t		zombie_children;	//Zombie child processes

    //Methods
    //pprocess_obj_t		fork(pprocess_obj_t p_this);
    struct _process_obj*	(*fork)(struct _process_obj*);

    //void					add_ref(pprocess_obj_t p_this, pproc_ref_obj_t p_ref_obj);
    void	(*add_ref_obj)(struct _process_obj*, pproc_ref_obj_t);

    //void					die(pprocess_obj_t p_this);
    void	(*die)(struct _process_obj*);

    //void	add_child(pprocess_obj_t p_this, u32 child_id);
    //void	zombie_child(pprocess_obj_t p_this, u32 child_id);
    //void	remove_child(pprocess_obj_t p_this, u32 child_id);
    //void	add_thread(pprocess_obj_t p_this, u32 thread_id);
    //void	zombie_thread(pprocess_obj_t p_this, u32 thread_id);
    //void	remove_thread(pprocess_obj_t p_this, u32 thread_id);

} process_obj_t, *pprocess_obj_t;
