/*
	  Copyright 2016,暗夜幽灵 <darknightghost.cn@gmail.com>

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

#include "../../../common/common.h"
#include "port.h"

//Interrupt level
#define		INT_LEVEL_HIGHEST		0xFF
#define		INT_LEVEL_LOWEST		0x00

#define		INT_LEVEL_EXCEPTION		0xFF
#define		INT_LEVEL_IO			0xD0
#define		INT_LEVEL_DISPATCH		0x40	//Dispatch messages and signals,the task will not switch.
#define		INT_LEVEL_USR_HIGHEST	0x0F
#define		INT_LEVEL_IDLE			0x00

void		interrupt_init();

void		io_enable_interrupt();
void		io_disable_interrupt();
