
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

#include "../../spinlock.h"

void pm_init_spn_lock(pspinlock_t p_lock)
{
	p_lock->owner = 0;
	p_lock->next = 0;
	p_lock->int_level = 0;

	return;
}

void pm_acqr_spn_lock(pspinlock_t p_lock)
{
	u32 ticket;

	//Get ticket
	__asm__ __volatile__(
	    "movl	$1,%%eax\n"
	    "lock	xaddl	%%eax,(%1)\n"
	    "movl	%%eax,%0\n"
	    :"=m"(ticket)
	    :"b"(&p_lock->next)
	    :"memory");

	//Get lock
	__asm__ __volatile__(
	    "_loop:\n"
	    "cmpl	(%0),%1\n"
	    "jne	_loop\n"
	    ::"b"(&p_lock->owner), "a"(ticket));

	return;
}

bool pm_try_acqr_spn_lock(pspinlock_t p_lock)
{
	bool ret;

	//Try to get lock
	__asm__ __volatile__(
	    "movl	%1,%%eax\n"
	    "movl	%%eax,%%edx\n"
	    "incl	%%edx\n"
	    "lock	cmpxchgl	%%edx,(%2)\n"
	    "jz		__JZ1\n"
	    "xorb	%0,%0\n"
	    "jmp	__TRYEND\n"
	    "__JZ1:\n"
	    "movb	$1,%0\n"
	    "__TRYEND:\n"
	    :"=a"(ret)
	    :"m"(p_lock->owner), "b"(&(p_lock->next))
	    :"memory");

	return ret;
}

void pm_rls_spn_lock(pspinlock_t p_lock)
{
	//Release spining lock
	p_lock->owner++;

	return;
}
