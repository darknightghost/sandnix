/*
	Copyright 2015,∞µ“π”ƒ¡È <darknightghost.cn@gmail.com>

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

#ifndef	INT_HANDLER_H_INCLUDE
#define	INT_HANDLER_H_INCLUDE

void	de_int_handler();
void	db_int_handler();
void	imr_int_handler();
void	bp_int_handler();
void	of_int_handler();
void	br_int_handler();
void	ud_int_handler();
void	nm_int_handler();
void	df_int_handler();
void	fpu_int_handler();
void	ts_int_handler();
void	np_int_handler();
void	ss_int_handler();
void	gp_int_handler();
void	pf_int_handler();
void	reserved_int_handler();
void	mf_int_handler();
void	ac_int_handler();
void	mc_int_handler();
void	xf_int_handler();

void	default_int_handler();

#endif	//!	INT_HANDLER_H_INCLUDE