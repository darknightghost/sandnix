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

#include "heap.h"
#include "../../exceptions/exceptions.h"

static	u8		kernel_default_heap[4096];
static	bool	default_heap_init_flag = false;


void* mm_heap_alloc(size_t size, void* heap_addr)
{
	u8* heap_base;
	pheap_head p_head;
	pmem_block_head p_block, p_new_block;
	u8* p_data;

	//Get the base addr of the heap
	if(heap_addr == NULL) {
		heap_base = kernel_default_heap;

		//Check if the heap has been initialized
		if(!default_heap_init_flag) {
			//Head
			p_head = (pheap_head)kernel_default_heap;
			p_head->p_prev = NULL;
			p_head->p_next = NULL;
			p_head->p_first_empty_block = (pmem_block_head)(p_head + 1);
			p_head->attr = HEAP_EXTENDABLE;
			//First block
			p_block = (pmem_block_head)(p_head + 1);
			p_block->magic = HEAP_INTACT_MAGIC;
			p_block->p_prev;
			p_block->p_next = NULL;
			p_block->p_prev_empty_block = NULL;
			p_block->p_next_empty_block = NULL;
			p_block->start_addr = p_block + 1;
			p_block->allocated_flag = false;
			p_block->size = (u32)kernel_default_heap - (u32)(p_block + 1) + sizeof(kernel_default_heap);

			default_heap_init_flag = true;
		}

	} else {
		heap_base = heap_addr;
	}

	p_block = p_head->p_first_empty_block;
	size = size % 4 ? (size / 4 + 1) * 4 : size;

	//Search for empty memory block
	while(true) {
		if(p_block == NULL) {
			if(p_head->p_next != NULL) {
				p_head = p_head->p_next;
				p_block = p_head->p_first_empty_block;
				continue;

			} else if((p_head->attr)&HEAP_EXTENDABLE) {
				//Allocate one page and initialize it
				return NULL;

			} else {
				return NULL;
			}
		}

		if(p_block->magic != HEAP_INTACT_MAGIC) {
			excpt_panic(EXCEPTION_BUF_OVERFLOW,
			            "The address of memory block is %p.",
			            p_block);
		}

		if(p_block->size >= size) {
			//Allocate memory
			if(p_block->size > size + sizeof(mem_block_head)) {
				//Spilt block
				p_new_block = (pmem_block_head)((u32)(p_block->start_addr) + size);
				p_new_block->p_next = p_block->p_next;
				p_new_block->p_prev = p_block;
				p_new_block->p_next_empty_block = p_block->p_next_empty_block;
				p_new_block->p_prev_empty_block = p_block;
				p_new_block->magic = HEAP_INTACT_MAGIC;
				p_new_block->start_addr = (void*)(p_new_block + 1);
				p_new_block->allocated_flag = false;
				p_new_block->size = (u32)p_block + p_block->size - (u32)p_new_block->start_addr;

				if(p_block->p_next != NULL) {
					p_block->p_next->p_prev = p_new_block;
				}

				if(p_block->p_next_empty_block != NULL) {
					p_block->p_next_empty_block->p_prev_empty_block = p_new_block;
				}

				p_block->p_next = p_new_block;
				p_block->p_next_empty_block = p_new_block;

			}

			//Allocate memory
			if(p_block->p_next_empty_block != NULL) {
				p_block->p_next_empty_block->p_prev_empty_block = p_block->p_prev_empty_block;
			}

			if(p_block->p_prev_empty_block != NULL) {
				p_block->p_prev_empty_block->p_next_empty_block = p_block->p_next_empty_block;

			} else {
				p_head->p_first_empty_block = p_block->p_next_empty_block;
			}

			p_block->allocated_flag = true;
			return p_block->start_addr;

		}

		p_block = p_block->p_next_empty_block;
	}
}

void mm_heap_free(void* addr, void* heap_addr)
{

}
