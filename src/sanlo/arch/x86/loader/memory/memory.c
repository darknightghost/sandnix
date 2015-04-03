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

#include "memory.h"
#include "../exception/exception.h"
#include "../io/stdout.h"

void init_heap()
{
	pmem_block_head p_first_head;
	p_first_head = HEAP_BASE;
	p_first_head->p_prev = NULL;
	p_first_head->p_next = NULL;
	p_first_head->start_addr = HEAP_BASE + sizeof(mem_block_head);
	p_first_head->allocated_flag = 0;
	p_first_head->size = HEAP_SIZE - sizeof(mem_block_head);
	return;
}

void* malloc(size_t size)
{
	pmem_block_head p_head;
	pmem_block_head p_prevhead;
	pmem_block_head p_new_head;
	p_head = HEAP_BASE;
	p_prevhead = NULL;
	size = (size % 4) ? ((size_t)(size / 4) + 1) * 4 : size;

	while(p_head != NULL) {
		//Check block head
		if(p_head < (pmem_block_head)HEAP_BASE
		   || p_head > (pmem_block_head)(HEAP_BASE + HEAP_SIZE)
		   || (p_head->p_prev != NULL && (p_head->p_prev < (pmem_block_head)HEAP_BASE
										  || p_head->p_prev > (pmem_block_head)(HEAP_BASE + HEAP_SIZE)))
		   || (p_head->p_next != NULL && (p_head->p_next < (pmem_block_head)HEAP_BASE
										  || p_head->p_next > (pmem_block_head)(HEAP_BASE + HEAP_SIZE))
			  )) {
			panic(EXCEPTION_HEAP_CORRUPTION);
		}

		if(!p_head->allocated_flag
		   && p_head->size >= size) {
			//Allocate memory
			if(p_head->size > size + sizeof(mem_block_head) + 4) {
				//Split memory block
				p_new_head = (pmem_block_head)((char*)(p_head->start_addr) + size);
				p_new_head->p_prev = p_head;
				p_new_head->p_next = p_head->p_next;
				p_new_head->start_addr = p_new_head + 1;
				p_new_head->size = p_head->size - size - sizeof(mem_block_head);
				p_new_head->allocated_flag = 0;

				if(p_head->p_next != NULL) {
					p_head->p_next->p_prev = p_new_head;
				}

				p_head->p_next = p_new_head;
				p_head->size = size;
			}

			p_head->allocated_flag = 1;

			if(p_head->start_addr > HEAP_BASE + HEAP_SIZE
			   || p_head->start_addr < HEAP_BASE) {
				panic(EXCEPTION_HEAP_CORRUPTION);
			}

			return p_head->start_addr;
		} else {
			//Get next memory block
			p_prevhead = p_head;
			p_head = p_head->p_next;
		}
	}
	print_string(
		GET_REAL_ADDR(
			"\nNOT ENOUGH MEMORY!!!.\n"),
		BG_BLACK | FG_BRIGHT_RED,
		BG_BLACK);
	return NULL;
}

void free(void* addr)
{
	pmem_block_head p_head;
	p_head = addr;
	p_head--;

	//Check block head
	if(p_head < (pmem_block_head)HEAP_BASE
	   || p_head > (pmem_block_head)(HEAP_BASE + HEAP_SIZE)
	   || (p_head->p_prev != NULL && (p_head->p_prev < (pmem_block_head)HEAP_BASE
									  || p_head->p_prev > (pmem_block_head)(HEAP_BASE + HEAP_SIZE)))
	   || (p_head->p_next != NULL && (p_head->p_next < (pmem_block_head)HEAP_BASE
									  || p_head->p_next > (pmem_block_head)(HEAP_BASE + HEAP_SIZE))
		  )) {
		panic(EXCEPTION_HEAP_CORRUPTION);
	}

	p_head->allocated_flag = 0;

	//Join memory blocks
	if(p_head->p_prev != NULL
	   && !(p_head->p_prev->allocated_flag)) {
		p_head->p_prev->size += p_head->size + sizeof(mem_block_head);
		p_head->p_prev->p_next = p_head->p_next;

		if(p_head->p_next != NULL) {
			p_head->p_next->p_prev = p_head->p_prev;
		}

		p_head = p_head->p_prev;
	}

	if(p_head->p_next != NULL
	   && !(p_head->p_next->allocated_flag)) {
		p_head = p_head->p_next;
		p_head->p_prev->size += p_head->size + sizeof(mem_block_head);
		p_head->p_prev->p_next = p_head->p_next;

		if(p_head->p_next != NULL) {
			p_head->p_next->p_prev = p_head->p_prev;
		}
	}

	return;
}
