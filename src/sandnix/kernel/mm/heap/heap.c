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

static	u8		kernel_default_heap[KERNEL_DEFAULT_HEAP_SIZE];
static	bool	default_heap_init_flag = false;


static	void	init_heap_head(void* start_addr, size_t scale,
                               u32 attr, pheap_head_t p_prev);
static	void	merge_mem_blocks(pheap_head_t p_head);
static	void	rebuild_empty_blocks_list(pheap_head_t p_head);


void* mm_hp_create(size_t max_block_size, u32 attr)
{
	void* addr;
	u32 scale;

	//Compute scale
	scale = ((max_block_size + sizeof(heap_head_t) + sizeof(mem_block_head_t)) / 4096
	         + ((max_block_size + sizeof(heap_head_t) + sizeof(mem_block_head_t)) % 4096 ? 1 : 0)) * 4096;

	//Allocate memory
	/*
	addr = mm_virt_alloc(NULL,
	                     scale,
	                     MEM_RESERVE | MEM_COMMIT,
	                     PAGE_WRITEABLE);
						 */

	//TODO:delete
	addr = NULL;

	//
	if(addr == NULL) {
		return NULL;
	}

	//Initialize the head
	init_heap_head(addr, scale, attr, NULL);
	return addr;
}

void mm_hp_destroy(void* heap_addr)
{
	pheap_head_t p_head, p_next_head;

	p_head = (pheap_head_t)heap_addr;

	while(p_head != NULL) {
		p_next_head = p_head->p_next;
		//mm_virt_free(p_head, p_head->scale, MEM_RELEASE);
		p_head = p_next_head;
	}

	return;
}

void* mm_hp_alloc(size_t size, void* heap_addr)
{
	pheap_head_t p_head, p_new_head;
	pmem_block_head_t p_block, p_new_block;
	pspinlock_t p_lock;

	//Get heap address
	if(heap_addr == NULL) {
		p_head = (pheap_head_t)kernel_default_heap;

		if(!default_heap_init_flag) {
			default_heap_init_flag = true;

			//Initialize default heap
			init_heap_head(kernel_default_heap, KERNEL_DEFAULT_HEAP_SIZE,
			               HEAP_EXTENDABLE | HEAP_MULTITHREAD, NULL);
		}

	} else {
		p_head = (pheap_head_t)heap_addr;
	}

	//Check if it is the first head of heap
	if(p_head->p_prev != NULL) {
		//panic
		excpt_panic(
		    EINVAL,
		    "The heap address which caused this problem is %p",
		    p_head);
	}

	size = size % 4 ? (size / 4 + 1) * 4 : size;

	if(size > p_head->scale - sizeof(mem_block_head_t) - sizeof(heap_head_t)) {
		return NULL;
	}

	//Allocate memory
	if(p_head->attr & HEAP_MULTITHREAD) {
		p_lock = &(p_head->lock);
		pm_acqr_spn_lock(p_lock);
	}

	while(1) {
		for(p_block = p_head->p_first_empty_block;
		    p_block != NULL;
		    p_block = p_block->p_next_empty_block) {

			//Check memory block
			if(p_block->magic != HEAP_INTACT_MAGIC) {
				//Panic
				excpt_panic(
				    EOVERFLOW,
				    "Heap corruption occured,the block which is broken might be %p",
				    p_block);
			}

			if(!p_block->allocated_flag
			   && p_block->size > size) {
				if(p_block->size > size + sizeof(mem_block_head_t)) {
					//Spilt memory block and allocate memory
					p_new_block = (pmem_block_head_t)((u8*)(p_block->start_addr) + size);
					p_new_block->magic = HEAP_INTACT_MAGIC;
					p_new_block->allocated_flag = false;
					p_new_block->start_addr = (u8*)(p_new_block + 1);
					p_new_block->size = p_block->size - size - sizeof(mem_block_head_t);
					p_block->size = size;

					p_block->allocated_flag = true;

					if(p_block->p_next_empty_block != NULL) {
						p_block->p_next_empty_block->p_prev_empty_block = p_new_block;
					}

					if(p_block->p_prev_empty_block != NULL) {
						p_block->p_prev_empty_block->p_next_empty_block = p_new_block;

					} else {
						p_head->p_first_empty_block = p_new_block;
					}

					p_new_block->p_next_empty_block = p_block->p_next_empty_block;
					p_new_block->p_prev_empty_block = p_block->p_prev_empty_block;

					if(p_head->attr & HEAP_MULTITHREAD) {
						pm_rls_spn_lock(p_lock);
					}

					return p_block->start_addr;

				} else {
					//Allocate memory
					p_block->allocated_flag = true;

					if(p_block->p_next_empty_block != NULL) {
						p_block->p_next_empty_block->p_prev_empty_block
						    = p_block->p_prev_empty_block;
					}

					if(p_block->p_prev_empty_block != NULL) {
						p_block->p_prev_empty_block->p_next_empty_block
						    = p_block->p_next_empty_block;

					} else {
						p_head->p_first_empty_block = p_block->p_next_empty_block;
					}

					if(p_head->attr & HEAP_MULTITHREAD) {
						pm_rls_spn_lock(p_lock);
					}

					return p_block->start_addr;
				}
			}

		}

		if(p_head->p_next == NULL) {
			if(p_head->attr & HEAP_EXTENDABLE) {
				//Allocate more pages
				excpt_panic(ENOMEM, "Un supported mm.\n");

				/*p_new_head = mm_virt_alloc(NULL, p_head->scale,
				MEM_RESERVE | MEM_COMMIT,
				            PAGE_WRITEABLE);
				*/
				//TODO:delete
				p_new_head = NULL;

				//
				if(p_new_head == NULL) {
					if(p_head->attr & HEAP_MULTITHREAD) {
						pm_rls_spn_lock(p_lock);
					}

					return NULL;
				}

				init_heap_head(p_new_head, p_head->scale, p_head->attr, p_head);
				p_head->p_next = p_new_head;

			} else {
				if(p_head->attr & HEAP_MULTITHREAD) {
					pm_rls_spn_lock(p_lock);
				}

				return NULL;
			}
		}

		p_head = p_head->p_next;
	}
}

void mm_hp_free(void* addr, void* heap_addr)
{
	pheap_head_t p_head;
	pmem_block_head_t p_block;
	pspinlock_t p_lock;

	//Get heap address
	if(heap_addr == NULL) {
		p_head = (pheap_head_t)kernel_default_heap;

	} else {
		p_head = (pheap_head_t)heap_addr;
	}

	p_block = (pmem_block_head_t)addr - 1;

	if(p_head->p_prev != NULL) {
		//Panic
		excpt_panic(
		    EINVAL,
		    "Illegal heap address,the heap address which caused this problem is %p",
		    p_head);
	}

	if(p_head->attr & HEAP_MULTITHREAD) {
		p_lock = &(p_head->lock);
		pm_acqr_spn_lock(p_lock);
	}

	if(p_block->magic != HEAP_INTACT_MAGIC) {
		//Panic
		excpt_panic(
		    EOVERFLOW,
		    "Heap corruption occured,the block which is broken might be %p",
		    p_block);
	}

	while((u8*)p_block < (u8*)(p_head + 1)
	      || (u8*)p_block > (u8*)p_head + p_head->scale) {
		if(p_head->p_next == NULL) {
			//Panic
			excpt_panic(
			    EINVAL,
			    "Illegal heap address,the heap address which caused this problem is %p",
			    heap_addr);
		}

		p_head = p_head->p_next;
	}

	//Free memory block
	p_block->allocated_flag = false;

	//Merge memory blocks
	merge_mem_blocks(p_head);

	if(p_head->p_prev != NULL
	   && p_head->p_first_empty_block != NULL
	   && (p_head->scale == p_head->p_first_empty_block->size
	       + sizeof(heap_head_t) + sizeof(mem_block_head_t))) {

		excpt_panic(ENOMEM, "Un supported mm.\n");

		//Free pages
		if(p_head->p_next != NULL) {
			p_head->p_next->p_prev = p_head->p_prev;
		}

		p_head->p_prev->p_next = p_head->p_next;
		//mm_virt_free(p_head, p_head->scale, MEM_RELEASE);
	}

	if(p_head->attr & HEAP_MULTITHREAD) {
		pm_rls_spn_lock(p_lock);
	}

	return;
}

void mm_hp_check(void* heap_addr)
{
	pheap_head_t p_head;
	pmem_block_head_t p_block;
	pspinlock_t p_lock;

	//Get heap address
	if(heap_addr == NULL) {
		p_head = (pheap_head_t)kernel_default_heap;

	} else {
		p_head = (pheap_head_t)heap_addr;
	}

	if(p_head->p_prev != NULL) {
		excpt_panic(
		    EINVAL,
		    "Illegal heap address,the heap address which caused this problem is %p",
		    heap_addr);
	}

	if(p_head->attr & HEAP_MULTITHREAD) {
		p_lock = &(p_head->lock);
		pm_acqr_spn_lock(p_lock);
	}

	while(p_head != NULL) {
		for(p_block = (pmem_block_head_t)(p_head + 1);
		    (u8*)p_block < (u8*)p_head + p_head->scale;
		    p_block = (pmem_block_head_t)(p_block->start_addr + p_block->size)) {
			if(p_block->magic != HEAP_INTACT_MAGIC) {
				//Panic
				excpt_panic(
				    EOVERFLOW,
				    "Heap corruption occured,the block which is broken might be %p",
				    p_block);
			}
		}

		p_head = p_head->p_next;
	}

	if(p_head->attr & HEAP_MULTITHREAD) {
		pm_rls_spn_lock(p_lock);
	}

	return;
}

void init_heap_head(void* start_addr, size_t scale, u32 attr, pheap_head_t p_prev)
{
	pheap_head_t p_head;
	pmem_block_head_t p_first_block;

	//Initialize head
	p_head = (pheap_head_t)start_addr;
	p_head->p_prev = p_prev;
	p_head->p_next = NULL;
	p_head->scale = scale;
	p_head->attr = attr;
	p_head->p_first_empty_block = (pmem_block_head_t)(p_head + 1);

	if(attr & HEAP_MULTITHREAD) {
		pm_init_spn_lock(&(p_head->lock));
	}

	//Initialize first block
	p_first_block = p_head->p_first_empty_block;
	p_first_block->magic = HEAP_INTACT_MAGIC;
	p_first_block->p_next_empty_block = NULL;
	p_first_block->p_prev_empty_block = NULL;
	p_first_block->allocated_flag = false;
	p_first_block->start_addr = (u8*)(p_first_block + 1);
	p_first_block->size = scale - sizeof(heap_head_t) - sizeof(mem_block_head_t);

	return;
}

void merge_mem_blocks(pheap_head_t p_head)
{
	pmem_block_head_t p_block;
	pmem_block_head_t p_next_block;

	//Merge memory blocks
	for(p_block = (pmem_block_head_t)(p_head + 1);
	    (u8*)p_block < (u8*)p_head + p_head->scale;
	    p_block = (pmem_block_head_t)(p_block->start_addr + p_block->size)) {
		if(p_block->magic != HEAP_INTACT_MAGIC) {
			//Panic
			excpt_panic(
			    EOVERFLOW,
			    "Heap corruption occured,the block which is broken might be %p",
			    p_block);
			return;
		}

		if(!p_block->allocated_flag) {
			for(p_next_block = (pmem_block_head_t)(p_block->start_addr + p_block->size);
			    (u8*)p_next_block < (u8*)p_head + p_head->scale
			    && !p_next_block->allocated_flag;
			    p_next_block = (pmem_block_head_t)(p_block->start_addr + p_block->size)
			   ) {
				if(p_next_block->magic != HEAP_INTACT_MAGIC) {
					//Panic
					excpt_panic(
					    EOVERFLOW,
					    "Heap corruption occured,the block which is broken might be %p",
					    p_block);
					return;
				}

				//Merge memory blocks
				(p_block->size) += p_next_block->size + sizeof(mem_block_head_t);
			}
		}
	}

	rebuild_empty_blocks_list(p_head);
	return;
}

void rebuild_empty_blocks_list(pheap_head_t p_head)
{
	pmem_block_head_t p_block, p_prev_empty_block;

	p_head->p_first_empty_block = NULL;

	//Rebuild list_t
	for(p_block = (pmem_block_head_t)(p_head + 1);
	    (u8*)p_block < (u8*)p_head + p_head->scale;
	    p_block = (pmem_block_head_t)(p_block->start_addr + p_block->size)) {
		if(p_block->magic != HEAP_INTACT_MAGIC) {
			//Panic
			excpt_panic(
			    EOVERFLOW,
			    "Heap corruption occured,the block which is broken might be %p",
			    p_block);
			return;
		}

		if(!p_block->allocated_flag) {
			//Add new block to the list_t
			if(p_head->p_first_empty_block == NULL) {
				p_block->p_prev_empty_block = NULL;
				p_block->p_next_empty_block = NULL;
				p_head->p_first_empty_block = p_block;

			} else {
				p_prev_empty_block->p_next_empty_block = p_block;
				p_block->p_prev_empty_block = p_prev_empty_block;
				p_block->p_next_empty_block = NULL;
			}

			p_prev_empty_block = p_block;
		}
	}

	return;
}