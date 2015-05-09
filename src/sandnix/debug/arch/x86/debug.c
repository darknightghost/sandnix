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

#include "../../debug.h"
#include "../../../mm/mm.h"
#include "../../../rtl/rtl.h"

static	char	k_dbg_tty_buf[K_TTY_BUF_SIZE];
static	char*	p_tty_buf = NULL;
static	bool	enable_print_flag = true;

static	void	buf_output();

void dbg_print(char* fmt, ...)
{
	char* buf;
	va_list args;
	size_t len;

	buf = mm_heap_alloc(1024, NULL);
	va_start(args, fmt);
	rtl_vprintf_s(buf, 1024, fmt, args);
	va_end(args);

	//Wrtie string to buf;
	if(p_tty_buf == NULL) {
		p_tty_buf = k_dbg_tty_buf;
	}

	len = rtl_strlen(buf);

	if(K_TTY_BUF_SIZE - (p_tty_buf - k_dbg_tty_buf) < len) {
		rtl_memmove(
		    k_dbg_tty_buf,
		    k_dbg_tty_buf + len - (K_TTY_BUF_SIZE - (p_tty_buf - k_dbg_tty_buf)),
		    len - (K_TTY_BUF_SIZE - (p_tty_buf - k_dbg_tty_buf)));
		p_tty_buf = p_tty_buf - (len - (K_TTY_BUF_SIZE - (p_tty_buf - k_dbg_tty_buf)));
	}

	rtl_memcpy(p_tty_buf, buf, len);
	mm_heap_free(buf, NULL);
	p_tty_buf += len;

	//Print string
	if(enable_print_flag) {
		buf_output();
	}

	return;
}

void buf_output()
{

}
