
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
#include "../../../../io/io.h"
#include "../../../../exceptions/exceptions.h"

void pm_init_spn_lock(pspin_lock p_lock)
{
	p_lock->owner = 0;
	p_lock->next = 0;
	p_lock->int_level = 0;

	return;
}

void pm_acqr_spn_lock(pspin_lock p_lock)
{
	u8 int_level;
	u32 ticket;

	int_level = io_get_crrnt_int_level();

	if(int_level < INT_LEVEL_DISPATCH) {
		io_set_crrnt_int_level(INT_LEVEL_DISPATCH);
	}

	//Get ticket
	__asm__ __volatile__(
	    "movl	$1,%%eax\n\t"
	    "lock	xaddl	%%eax,(%1)\n\t"
	    "movl	%%eax,%0\n\t"
	    :"=m"(ticket)
	    :"b"(&p_lock->next));

	//Get lock
	while(p_lock->owner != ticket);

	//Increase interrupt level
	p_lock->int_level = int_level;

	return;
}

void pm_acqr_raw_spn_lock(pspin_lock p_lock)
{
	u32 ticket;

	//Get ticket
	__asm__ __volatile__(
	    "movl	$1,%%eax\n\t"
	    "lock	xaddl	%%eax,(%1)\n\t"
	    "movl	%%eax,%0\n\t"
	    :"=m"(ticket)
	    :"b"(&p_lock->next));

	//Get lock
	while(p_lock->owner != ticket);

	return;
}

bool pm_try_acqr_spn_lock(pspin_lock p_lock)
{

	u8 int_level;
	bool ret;

	int_level = io_get_crrnt_int_level();

	if(int_level < INT_LEVEL_DISPATCH) {
		//Increase interrupt level
		io_set_crrnt_int_level(INT_LEVEL_DISPATCH);
	}

	//Try to get lock
	__asm__ __volatile__(
	    "movl	%1,%%eax\n\t"
	    "movl	%%eax,%%edx\n\t"
	    "incl	%%edx\n\t"
	    "lock	cmpxchgl	%%edx,(%2)\n\t"
	    "jz		_JZ1\n\t"
	    "xorl	%0,%0\n\t"
	    "jmp	_TRYEND\n\t"
	    "_JZ1:\n\t"
	    "movl	$1,%0\n\t"
	    "_TRYEND:\n\t"
	    :"=a"(ret)
	    :"m"(p_lock->owner), "b"(&(p_lock->next)));

	if(ret) {
		p_lock->int_level = int_level;

	} else {
		io_set_crrnt_int_level(int_level);
	}


	return ret;
}

bool pm_try_acqr_raw_spn_lock(pspin_lock p_lock)
{
	bool ret;

	//Try to get lock
	__asm__ __volatile__(
	    "movl	%1,%%eax\n\t"
	    "movl	%%eax,%%edx\n\t"
	    "incl	%%edx\n\t"
	    "lock	cmpxchgl	%%edx,(%2)\n\t"
	    "jz		__JZ1\n\t"
	    "xorl	%0,%0\n\t"
	    "jmp	__TRYEND\n\t"
	    "__JZ1:\n\t"
	    "movl	$1,%0\n\t"
	    "__TRYEND:\n\t"
	    :"=a"(ret)
	    :"m"(p_lock->owner), "b"(&(p_lock->next)));

	return ret;
}

void pm_rls_spn_lock(pspin_lock p_lock)
{
	//Release spining lock
	p_lock->owner++;

	//Decrease interrupt level
	io_set_crrnt_int_level(p_lock->int_level);

	return;

}

void pm_rls_raw_spn_lock(pspin_lock p_lock)
{
	//Release spining lock
	p_lock->owner++;

	return;

}
