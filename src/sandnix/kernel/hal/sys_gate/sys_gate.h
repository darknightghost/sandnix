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

#include "../../../../common/common.h"

#include "./sys_gate_defs.h"

//Initialize
void hal_sys_gate_init();

//Initialize cpu core
void hal_sys_gate_core_init();

//Set kernel entery
void hal_sys_gate_set_entry(void* entry);

//Return to user memory
void hal_sys_gate_ret(
    pcontext_t p_context,
    address_t ret);	//User space context

//Switch to user mode
void hal_sys_gate_go_to_usr(
    sys_gate_entry_t entry,	//Address
    int argc,				//Number of arguments
    char* argv[],			//Arguments
    char* env[]);			//Environment
