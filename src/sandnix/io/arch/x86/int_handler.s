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

.global	de_int_handler
.global	db_int_handler
.global	imr_int_handler
.global	bp_int_handler
.global	of_int_handler
.global	br_int_handler
.global	ud_int_handler
.global	nm_int_handler
.global	df_int_handler
.global	fpu_int_handler
.global	ts_int_handler
.global	np_int_handler
.global	ss_int_handler
.global	gp_int_handler
.global	pf_int_handler
.global	reserved_int_handler
.global	mf_int_handler
.global	ac_int_handler
.global	mc_int_handler
.global	xf_int_handler
.global	default_int_handler

.section	.text
.code32

de_int_handler:

db_int_handler:

imr_int_handler:

bp_int_handler:

of_int_handler:

br_int_handler:

ud_int_handler:

nm_int_handler:

df_int_handler:

fpu_int_handler:

ts_int_handler:

np_int_handler:

ss_int_handler:

gp_int_handler:

pf_int_handler:

reserved_int_handler:

mf_int_handler:

ac_int_handler:

mc_int_handler:

xf_int_handler:

default_int_handler:
			iret