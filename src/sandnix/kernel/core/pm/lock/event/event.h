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

#include "../../../../../../common/common.h"
#include "./event_defs.h"

//Initialize event
void		core_pm_event_init(pevent_t p_event);

//Wait for event
kstatus_t	core_pm_event_wait(pevent_t p_event, s32 millisec_timeout);

//Set event
void		core_pm_event_set(pevent_t p_event, bool broadcast);

//Destroy event
void		core_pm_event_destroy(pevent_t p_event);
