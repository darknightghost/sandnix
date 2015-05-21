
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

#include "spinlock.h"
#include "../../../io/io.h"
#include "../../../exceptions/exceptions.h"

void pm_init_spn_lock(pspin_lock p_lock)
{
	p_lock->lock = 0;
	p_lock->int_level = 0;

	return;
}

void pm_acqr_spn_lock(pspin_lock p_lock)
{
	if(io_get_crrnt_int_level() > INT_LEVEL_DISPATCH) {
		excpt_panic(EXCEPTION_INT_LEVEL_ERROR,
		            "Spining locks can only be used while interrupt level <= INT_LEVEL_DISPATCH\n");
	}

	return;
}

bool pm_try_acqr_spn_lock(pspin_lock p_lock)
{
	if(io_get_crrnt_int_level() > INT_LEVEL_DISPATCH) {
		excpt_panic(EXCEPTION_INT_LEVEL_ERROR,
		            "Spining locks can only be used while interrupt level <= INT_LEVEL_DISPATCH\n");
	}

	return false;
}

void pm_rls_spn_lock(p_lock)
{
	if(io_get_crrnt_int_level() > INT_LEVEL_DISPATCH) {
		excpt_panic(EXCEPTION_INT_LEVEL_ERROR,
		            "Spining locks can only be used while interrupt level <= INT_LEVEL_DISPATCH\n");
	}

	p_lock->lock = 0;
	io_set_crrnt_int_level(p_lock->int_level);
	return;
}
