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

#include "../../../../../../common/common.h"
#include "../msg_queue_obj_defs.h"
#include "./msg_obj_defs.h"
#include "../../../mm/mm.h"

pmsg_obj_t	msg_obj(
    u32 major_type,
    u32 minor_type,
    u32 attr,
    pmsg_queue_obj_t p_reply_queue,
    size_t size,
    pheap_t heap);