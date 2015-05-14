/*
	Copyright 2015,暗夜幽灵 <darknightghost.cn@gmail.com>

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

#include "../../exceptions.h"

static	const	char*		excpt_tbl[] = {
	"EXCEPTION_UNKNOW",
	"EXCEPTION_UNHANDLED_DE",
	"EXCEPTION_UNHANDLED_DB",
	"EXCEPTION_UNHANDLED_BP",
	"EXCEPTION_UNHANDLED_OF",
	"EXCEPTION_UNHANDLED_BR",
	"EXCEPTION_UNHANDLED_UD",
	"EXCEPTION_UNHANDLED_NM",
	"EXCEPTION_UNHANDLED_DF",
	"EXCEPTION_UNHANDLED_FPU",
	"EXCEPTION_UNHANDLED_TS",
	"EXCEPTION_UNHANDLED_NP",
	"EXCEPTION_UNHANDLED_SS",
	"EXCEPTION_UNHANDLED_GP",
	"EXCEPTION_UNHANDLED_PF",
	"EXCEPTION_UNHANDLED_MF",
	"EXCEPTION_UNHANDLED_AC",
	"EXCEPTION_UNHANDLED_MC",
	"EXCEPTION_UNHANDLED_XF",
	"EXCEPTION_INT_LEVEL_ERROR",
	"EXCEPTION_IMT_NUM_TOO_LARGE",
	"EXCEPTION_BUF_OVERFLOW",
	"EXCEPTION_HEAP_CORRUPTION"
	"EXCEPTION_ILLEGAL_HEAP_ADDR",
	"EXCEPTION_UNSPECIFIED_ROOT_PARTITION"
};


void	excpt_panic(u32 reason, char* fmt, ...)
{
	//Disable interrupt
	__asm__ __volatile__(
	    "cli\n\t"
	);

	//Reset video card to text mode

	//Clear screen

	//Print infomations of exceptions

	while(1);

	//Never return,in fact.
	return;
}
